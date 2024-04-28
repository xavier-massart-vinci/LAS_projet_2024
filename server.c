#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

#include <sys/ipc.h>
#include <sys/sem.h>

#include "utils_v1.h"
#include "messages.h"
#include "network.h"
#include "jeu.h"
#include "ipc.h"


/*** globals variables ***/
Client tabClients[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;
void childProcess(void *arg1);



void endServerHandler(int sig)
{
    end_inscriptions = 1;
}

void disconnect_players(Client *tabClients, int nbPlayers)
{
    for (int i = 0; i < nbPlayers; i++) {
        sclose(tabClients[i].sockfd);
    }

    clearSharedMemory();
    return;
}

int main(int argc, char const *argv[])
{
    // create IPCs
    semInit();
    shmInit();

    int sockfd, newsockfd, i;
    int nbPLayers = 0;

    Message msg;

    ssigaction(SIGALRM, endServerHandler);

    sockfd = initSocketServer(SERVER_PORT);
    printf("Le serveur tourne sur le port : %i \n", SERVER_PORT);

    i = 0;

    // INSCRIPTION PART
    alarm(TIME_INSCRIPTION);

    while (!end_inscriptions)
    {
        /* client trt */
        newsockfd = accept(sockfd, NULL, NULL); // saccept() exit le programme si accept a été interrompu par l'alarme

        if (newsockfd > 0) /* no error on accept */
        {
            sread(newsockfd, &msg, sizeof(msg));

            if (msg.code == INSCRIPTION_REQUEST)
            {
                printf("Inscription demandée par le joueur : %s\n", msg.pseudo);

                addPlayer(msg.pseudo);

                tabClients[i].sockfd = newsockfd;
                i++;

                if (nbPLayers < MAX_PLAYERS)
                {
                    msg.code = INSCRIPTION_OK;
                    nbPLayers++;
                    if (nbPLayers == MAX_PLAYERS)
                    {
                        alarm(0); // cancel alarm
                        end_inscriptions = 1;
                    }
                }
                else
                {
                    msg.code = INSCRIPTION_KO;
                }
                swrite(newsockfd, &msg, sizeof(msg));
                printf("Nb Inscriptions : %i\n", nbPLayers);
            }
        }
    }


    printf("FIN DES INSCRIPTIONS\n");
    if (nbPLayers < 2)
    {
        printf("PARTIE ANNULEE .. PAS ASSEZ DE JOUEURS\n");
        msg.code = CANCEL_GAME;
        for (i = 0; i < nbPLayers; i++)
        {
            swrite(tabClients[i].sockfd, &msg, sizeof(msg));
        }
        disconnect_players(tabClients, nbPLayers);
        sclose(sockfd);
        exit(0);
    }
    else
    {
        struct pollfd fds[MAX_PLAYERS];

        printf("PARTIE VA DEMARRER ... \n");
        msg.code = START_GAME;
        for (i = 0; i < nbPLayers; i++)
            swrite(tabClients[i].sockfd, &msg, sizeof(msg));

        // pour tout les clients
        for (int i = 0; i < nbPLayers; ++i)
        {
           // Création des pipes
           // Parent: parent -> child
           spipe(tabClients[i].pipefdParent);

           // Child: child -> parent
           spipe(tabClients[i].pipefdChild);

            // Création d'un processus fils
            pid_t childPid = fork_and_run1(childProcess, &tabClients[i]);

            // Configuration des pipes
            sclose(tabClients[i].pipefdParent[0]);
            sclose(tabClients[i].pipefdChild[1]);
            tabClients[i].childPid = childPid;

            // add sock to fds poll
            fds[i].fd = tabClients[i].pipefdChild[0];
            fds[i].events = POLLIN;
        }

        // creation des tuiles (tableau ) !!! 40 = joker (tab commence à 0) !!!
        int tilesTab[TILES_TAB_SIZE];
        createTiles(tilesTab);

        int nbPlayersScoreSended = 0;

        for (int i = 0; i < NB_ROUND; ++i)
        {
            msg.code = TUILE_PIOCHEE;
            msg.tileTake = getRandomTile(tilesTab, TILES_TAB_SIZE - i);

            for (int j = 0; j < nbPLayers; ++j)
            {
                // send TUILE_PIOCHEE
                swrite(tabClients[j].pipefdParent[1], &msg, sizeof(msg));
            }

            int nbPlayersPlayed = 0;
            while (nbPlayersPlayed != nbPLayers) {
                spoll(fds, nbPLayers, 0);

                for (int j = 0; j < nbPLayers; ++j)
                {
                    if (fds[j].revents & POLLIN) {
                        // wait TUILE_PLACEE
                        sread(tabClients[j].pipefdChild[0], &msg, sizeof(msg)); 
                        printf("%d\n", msg.tileTake);
                        if(msg.code == TUILE_PLACEE){
                            nbPlayersPlayed++;
                        }


                        // recuppere les score envoyer quand tout le monde a pas envoyer sont derniere tuile placer
                        if(msg.code == SCORE){
                            addPlayerScore(j, msg.playerScore);
                            nbPlayersScoreSended++;

                        }
                        
                    }
                }
            }

        }

        /// ------------------------------------------------ ///

        printf("%d\n", nbPLayers);
        
        // reccupere le rest des scores 
        while (nbPlayersScoreSended != nbPLayers) {
            poll(fds, nbPLayers, 0);

            for (int j = 0; j < nbPLayers; ++j)
            {
                if (fds[j].revents & POLLIN) {
                    sread(tabClients[j].pipefdChild[0], &msg, sizeof(msg)); 
                    addPlayerScore(j, msg.playerScore);
                    nbPlayersScoreSended++;
                }
            }
        }

        printf("CALCULE DES SCORE ... \n");
        sortPlayerScore();

        for (int i = 0; i < nbPLayers; ++i)
        {
            swait(NULL);
        }

        clearSharedMemory();
        
    }
    
    return 0;
}

void childProcess(void *arg1)
{
    Client *client = arg1;

    client->childPid = getpid();

    // Configuration des pipes
    sclose(client->pipefdParent[1]);
    sclose(client->pipefdChild[0]);

    Message msg;


    // Game Loop
    for (int i = 0; i < NB_ROUND; ++i)
    {
        // wait TUILE_PIOCHEE
        sread(client->pipefdParent[0], &msg, sizeof(msg));
        // send TUILE_PIOCHEE
        swrite(client->sockfd, &msg, sizeof(msg));
        // wait TUILE_PLACEE
        sread(client->sockfd, &msg, sizeof(msg));
        // send TUILE_PLACEE
        swrite(client->pipefdChild[1], &msg, sizeof(msg));
    }



    // Score 
    
    // wait SCORE
    sread(client->sockfd, &msg, sizeof(msg));
    // send SCORE
    swrite(client->pipefdChild[1], &msg, sizeof(msg)); 


    // send RANK
    TabPlayer* tabPlayer = readTabPlayer();

    msg.tabPlayer = *tabPlayer;
    swrite(client->sockfd, &msg, sizeof(msg));

    sshmdt(tabPlayer);

}

