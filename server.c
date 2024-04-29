#include "ipc.h"
#include "jeu.h"
#include "network.h"

/*** globals variables ***/
Client tabClients[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;
volatile sig_atomic_t end = 0;

void childProcess(void *arg1);
void setupTiles(int* tilesTab, int* readedLine);
void endServerHandler(int sig);
void disconnect_players(Client *tabClients, int nbPlayers);
void endServerHandlerEnd(int sig);
void initClients(Client* clients, int nbPlayers, struct pollfd *fds, Message *msg);
void initSig(sigset_t *set);


int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("error of use : %s [port] \n", argv[0]);
        exit(1);
    }

    int tilesTab[TILES_TAB_SIZE];
    int readedLine = 0;
    int port = atoi(argv[1]);

    int sockfd = initSocketServer(port);
    
    createIPC();

    sigset_t set;
    initSig(&set);

    while (!end)
    {
        ssigprocmask(SIG_BLOCK, &set, NULL);

        end_inscriptions = 0;
        Message msg;
        int newsockfd, i = 0;
        int nbPlayers = 0;

        // reset IPC
        resetSHM();

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

                    if (nbPlayers < MAX_PLAYERS)
                    {
                        msg.code = INSCRIPTION_OK;
                        nbPlayers++;
                        if (nbPlayers == MAX_PLAYERS)
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
                    printf("Nb Inscriptions : %i\n", nbPlayers);
                }
            }
        }

        printf("FIN DES INSCRIPTIONS\n");
        if (nbPlayers < 2)
        {
            printf("PARTIE ANNULEE .. PAS ASSEZ DE JOUEURS\n");
            msg.code = CANCEL_GAME;
            for (i = 0; i < nbPlayers; i++)
            {
                swrite(tabClients[i].sockfd, &msg, sizeof(msg));
            }
            disconnect_players(tabClients, nbPlayers);
        }
        else
        {
            setupTiles(tilesTab, &readedLine);

            struct pollfd fds[MAX_PLAYERS];

            printf("PARTIE VA DEMARRER ... \n");
            msg.code = START_GAME;
            

            initClients(tabClients, nbPlayers, fds, &msg);
            

            // Main
            // current game round
            int currentRound = 0;
            // the number of the current player have playeing in the current round
            int currentPlayerPlayed = 0;
            // the number of player (who have send they score)
            int nbScoreSended = 0;

            // init the first round
            sendTile(tabClients, nbPlayers, tilesTab[currentRound]);

            while (currentRound != NB_ROUND || nbScoreSended != nbPlayers)
            {
                spoll(fds, nbPlayers, 0);

                for (int i = 0; i < nbPlayers; ++i)
                {
                    // listenner of filedescriptor
                    if (fds[i].revents & POLLIN)
                    {
                        // read the current reauets message
                        sread(tabClients[i].pipefdChild[0], &msg, sizeof(msg));

                        if (msg.code == TUILE_PLACEE)
                        {
                            currentPlayerPlayed++;

                            // end of the current round
                            if (currentPlayerPlayed == nbPlayers)
                            {
                                currentRound++;
                                currentPlayerPlayed = 0;
                                sendTile(tabClients, nbPlayers, tilesTab[currentRound]);
                            }
                        }
                        // read score form player
                        else if (msg.code == SCORE)
                        {
                            addPlayerScore(i, msg.playerScore);
                            nbScoreSended++;
                        }
                    }
                }
            }

            printf("Génrération du leaderboard ... \n");
            sortPlayerScore();

            // wait every child
            for (int i = 0; i < nbPlayers; ++i)
            {
                swait(NULL);
            }

            // clean all pipe
            for (int i = 0; i < nbPlayers; ++i)
            {
                sclose(tabClients[i].pipefdParent[1]);
                sclose(tabClients[i].pipefdChild[0]);
            }
        }

        ssigprocmask(SIG_UNBLOCK, &set, NULL);
    }

    sclose(sockfd);
    clearSHM();

    return 0;
}

void childProcess(void *arg1)
{
    Client *client = arg1;
    Message msg;

    sclose(client->pipefdParent[1]);
    sclose(client->pipefdChild[0]);

    // Game Loop
    for (int i = 0; i < NB_ROUND; ++i)
    {
        // read and send random tile
        sread(client->pipefdParent[0], &msg, sizeof(msg));
        swrite(client->sockfd, &msg, sizeof(msg));

        // read and send tile placed
        sread(client->sockfd, &msg, sizeof(msg));
        swrite(client->pipefdChild[1], &msg, sizeof(msg));
    }

    // read and send player score
    sread(client->sockfd, &msg, sizeof(msg));
    swrite(client->pipefdChild[1], &msg, sizeof(msg));

    // read and send leaderboard
    TabPlayer *tabPlayer = readTabPlayer();
    msg.tabPlayer = *tabPlayer;
    swrite(client->sockfd, &msg, sizeof(msg));

    // close
    sshmdt(tabPlayer);
    sclose(client->sockfd);
    sclose(client->pipefdChild[1]);
    sclose(client->pipefdParent[0]);
}

void setupTiles(int* tilesTab, int* readedLine)
{
    char *currentLigne;

    // when tiles is gave
    if ((currentLigne = readLine()) != NULL)
    {
        for (int i = 0; i < NB_ROUND; i++)
        {
            (*readedLine)++;
            tilesTab[i] = atoi(currentLigne);
            currentLigne = readLine();
        }
    }
    else
    {
        // normal gen
        int defautTiles[TILES_TAB_SIZE];
        createTiles(defautTiles);

        for (size_t i = 0; i < NB_ROUND; i++)
        {
            defautTiles[i] = getRandomTile(defautTiles, TILES_TAB_SIZE - i);
        }
    }
}


void endServerHandler(int sig)
{
    end_inscriptions = 1;
}

void disconnect_players(Client *tabClients, int nbPlayers)
{
    for (int i = 0; i < nbPlayers; i++)
    {
        sclose(tabClients[i].sockfd);
    }
}

void endServerHandlerEnd(int sig)
{
    end = 1;
}


void initClients(Client* clients, int nbPlayers, struct pollfd* fds, Message *msg){
    // pour tout les clients
    for (int i = 0; i < nbPlayers; ++i)
    {
        // envoie début de partie à tabClients[i]
        swrite(tabClients[i].sockfd, &msg, sizeof(msg));
        
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

}
    

void initSig(sigset_t* set){
    ssigemptyset(set);
    sigaddset(set, SIGINT);

    ssigaction(SIGALRM, endServerHandler);
    ssigaction(SIGINT, endServerHandlerEnd); 
}