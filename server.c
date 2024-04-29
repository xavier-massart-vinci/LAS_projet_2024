/**
 * Authors: Debin Pierre-Alexandre, Devos Thibaut, Massart Xavier
 */

#include "ipc.h"
#include "jeu.h"
#include "network.h"

/*** globals variables ***/
Client tabClients[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;
volatile sig_atomic_t end = 0;

void childProcess(void *arg1);
void setupTiles(int *tilesTab);
void endServerHandler(int sig);
void disconnect_players(Client *tabClients, int nbPlayers);
void endServerHandlerEnd(int sig);
void initClients(Client *clients, int nbPlayers, struct pollfd *fds, Message *msg);
void initSig(sigset_t *set);

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("error of use : %s [port] \n", argv[0]);
        exit(1);
    }

    int tilesTab[TILES_TAB_SIZE];
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

        resetSHM();

        alarm(TIME_INSCRIPTION);

        while (!end_inscriptions)
        {
            newsockfd = accept(sockfd, NULL, NULL); 

            if (newsockfd > 0) 
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
                            alarm(0); 
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
            setupTiles(tilesTab);

            struct pollfd fds[MAX_PLAYERS];

            printf("PARTIE VA DEMARRER ... \n");
            msg.code = START_GAME;

            initClients(tabClients, nbPlayers, fds, &msg);

            int currentRound = 0;
            int currentPlayerPlayed = 0;
            int nbScoreSended = 0;

            sendTile(tabClients, nbPlayers, tilesTab[currentRound]);

            while (currentRound != NB_ROUND || nbScoreSended != nbPlayers)
            {
                spoll(fds, nbPlayers, 0);

                for (int i = 0; i < nbPlayers; ++i)
                {
                    if (fds[i].revents & POLLIN)
                    {
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
                        else if (msg.code == SCORE)
                        {
                            addPlayerScore(i, msg.playerScore);
                            nbScoreSended++;
                        }
                    }
                }
            }

            printf("Génération du leaderboard ... \n");
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

/**
 * PRE:  arg1: a pointer to a Client object
 * POST: start the child process of the server
 */
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

    sshmdt(tabPlayer);
    sclose(client->sockfd);
    sclose(client->pipefdChild[1]);
    sclose(client->pipefdParent[0]);
}

/**
 * PRE:  tilesTab: a pointer to an array of integers
 * POST: generates the tiles for the game
 */
void setupTiles(int *tilesTab)
{
    char* currentLine = readLine();

    if (currentLine != NULL && isatty(STDIN_FILENO) == 0) 
    {
        tilesTab[0] = atoi(currentLine);
        free(currentLine);
        for (int i = 1; i < NB_ROUND; i++)
        {
            currentLine = readLine();
            tilesTab[i] = atoi(currentLine);
            free(currentLine);
        }
    }else{
        free(currentLine);

        int defautTiles[TILES_TAB_SIZE];
        createTiles(defautTiles);

        for (int i = 0; i < NB_ROUND; i++)
        {
            tilesTab[i] = getRandomTile(defautTiles, TILES_TAB_SIZE - i);
        }
    }
    
}

/**
 * PRE:  sig: a signal
 * POST: set end_inscriptions to 1
 
 */
void endServerHandler(int sig)
{
    end_inscriptions = 1;
}

/**
 * PRE:  tabClients: a pointer to an array of Client structures
 *       nbPlayers: the number of players
 * POST: disconnects all players
 */
void disconnect_players(Client *tabClients, int nbPlayers)
{
    for (int i = 0; i < nbPlayers; i++)
    {
        sclose(tabClients[i].sockfd);
    }
}

/**
 * PRE:  sig: a signal
 * POST: set end to 1
 */
void endServerHandlerEnd(int sig)
{
    end = 1;
}

/**
 * PRE:  clients: a pointer to an array of Client structures
 *       nbPlayers: the number of players
 *       fds: a pointer to an array of pollfd structures
 *       msg: a pointer to a Message structure
 * POST: initializes the clients
 */
void initClients(Client *clients, int nbPlayers, struct pollfd *fds, Message *msg)
{
    for (int i = 0; i < nbPlayers; ++i)
    {
        // send start game to tabClients[i]
        swrite(tabClients[i].sockfd, &msg, sizeof(msg));

        // Parent: parent -> child
        spipe(tabClients[i].pipefdParent);

        // Child: child -> parent
        spipe(tabClients[i].pipefdChild);

        pid_t childPid = fork_and_run1(childProcess, &tabClients[i]);

        sclose(tabClients[i].pipefdParent[0]);
        sclose(tabClients[i].pipefdChild[1]);
        tabClients[i].childPid = childPid;

        // add sock to fds poll
        fds[i].fd = tabClients[i].pipefdChild[0];
        fds[i].events = POLLIN;
    }
}

/**
 * PRE:  set: a pointer to a sigset_t structure
 * POST: initializes the signal set
 */
void initSig(sigset_t* set)
{
    ssigemptyset(set);
    sigaddset(set, SIGINT);

    ssigaction(SIGALRM, endServerHandler);
    ssigaction(SIGINT, endServerHandlerEnd);
}
