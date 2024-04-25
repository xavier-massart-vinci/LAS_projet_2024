#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

#include "utils_v1.h"
#include "messages.h"
#include "network.h"
#include "jeu.h"


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
    return;
}

int main(int argc, char const *argv[])
{
    int sockfd, newsockfd, i;
    Message msg;
   // struct pollfd fds[MAX_PLAYERS];
    // char winnerName[256];

    ssigaction(SIGALRM, endServerHandler);

    sockfd = initSocketServer(SERVER_PORT);
    printf("Le serveur tourne sur le port : %i \n", SERVER_PORT);

    i = 0;
    int nbPLayers = 0;

    // INSCRIPTION PART
    alarm(TIME_INSCRIPTION);

    while (!end_inscriptions)
    {
        /* client trt */
        newsockfd = accept(sockfd, NULL, NULL); // saccept() exit le programme si accept a été interrompu par l'alarme
        if (newsockfd > 0)                      /* no error on accept */
        {

            sread(newsockfd, &msg, sizeof(msg));

            if (msg.code == INSCRIPTION_REQUEST)
            {
                printf("Inscription demandée par le joueur : %s\n", msg.pseudo);

                // strcpy(tabClients[i].pseudo, msg.messageText);
                // Shared Memory push
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
        printf("PARTIE VA DEMARRER ... \n");
        msg.code = START_GAME;
        for (i = 0; i < nbPLayers; i++)
            swrite(tabClients[i].sockfd, &msg, sizeof(msg));

        // pour tout les clients
        for (int i = 0; i < nbPLayers; ++i)
        {
           // Création des pipes
           // Parent: parent -> child
           spipe(tabClients[i].pipefdEcriture);

           // Child: child -> parent
           spipe(tabClients[i].pipefdLecture);

            // Création d'un processus fils
            pid_t childPid = fork_and_run1(childProcess, &tabClients[i]);

            // Configuration des pipes
            sclose(tabClients[i].pipefdEcriture[0]);
            sclose(tabClients[i].pipefdLecture[1]);
            tabClients[i].childPid = childPid;
        }


        // creation des tuiles (tableau ) !!! 40 = joker (tab commence à 0) !!!
        int tilesTab[TILES_TAB_SIZE];
        createTiles(tilesTab);

        for (int i = 0; i < NB_ROUND; ++i)
        {
            getRandomTile(tilesTab, TILES_TAB_SIZE - i);

        }


        // tirage d'une tuile au hasard
            // envoie de la tuile a tout les fils via le pipe

        // attente que tout les fils on envoyer leur répose

        // boucle 20 x sur ligne 123

    }
    
    return 0;
}

void childProcess(void *arg1)
{
    Client *client = arg1;

    client->childPid = getpid();

   // Configuration des pipes
   sclose(client->pipefdEcriture[1]);
   sclose(client->pipefdLecture[0]);
}

//points