#include "ipc.h"
#include "jeu.h"
#include "network.h"

/*** globals variables ***/
Client tabClients[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;
volatile sig_atomic_t end = 0;
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

void endServerHandlerEnd(int sig)
{
    end = 1;
}



int main(int argc, char const *argv[])
{
    /*
    if(argc < 2){
        printf("error of use : %s [port] \n", argv[0]);
        exit(1);
    }
    */
    // creation des tuiles (tableau ) !!! 40 = joker (tab commence à 0) !!!
    int tilesTab[TILES_TAB_SIZE];


    if(argc == 3){
        for (int i = 0; i < NB_ROUND; ++i)
        {
            char* currentLigne = readLine();
            tilesTab[0] = atoi(currentLigne);
        }
    }

    int port = SERVER_PORT;// atoi(argv[1]);

    // create IPCs
    semInit();
    shmInit();

    
    sigset_t set;
    ssigemptyset(&set);
    sigaddset(&set, SIGINT);
    ssigprocmask(SIG_BLOCK, &set, NULL);


    ssigaction(SIGALRM, endServerHandler);
    ssigaction(SIGINT, endServerHandlerEnd);

    int sockfd = initSocketServer(port);
    printf("Le serveur tourne sur le port : %i \n", port);



    while(!end){

        // when no file is give
        if(argc != 3){
            int defautlTiles[TILES_TAB_SIZE];
            createTiles(defautlTiles);

            int indexTilesTab = 0;

            for (int i = 0; i < NB_ROUND; ++i)
            {
                tilesTab[indexTilesTab] = getRandomTile(defautlTiles, TILES_TAB_SIZE - i);
                printf("TAB create => %d \n", tilesTab[indexTilesTab]);
                indexTilesTab++;
            }
        }


        end_inscriptions = 0;
        Message msg;
        int newsockfd, i = 0;
        int nbPLayers = 0;


        ssigprocmask(SIG_BLOCK, &set, NULL);


        // TODO reset IPC 
        cleanSHM();



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





            // Main
            // current game round
            int currentRound = 0;
            // the number of the current player have playeing in the current round
            int currentPlayerPlayed = 0;
            // the number of player (who have send they score)
            int nbScoreSended = 0;


            // init the first round
            sendTile(tabClients, nbPLayers, tilesTab[currentRound]);
            
            while (currentRound != NB_ROUND || nbScoreSended != nbPLayers){
                spoll(fds, nbPLayers, 0);

                for (int i = 0; i < nbPLayers; ++i)
                {
                    // listenner of filedescriptor
                    if (fds[i].revents & POLLIN) {
                        // read the current reauets message
                        sread(tabClients[i].pipefdChild[0], &msg, sizeof(msg)); 


                        if(msg.code == TUILE_PLACEE){
                            currentPlayerPlayed++;

                            // end of the current round
                            if(currentPlayerPlayed == nbPLayers){
                                currentRound++;
                                currentPlayerPlayed = 0;
                                sendTile(tabClients, nbPLayers, tilesTab[currentRound]);
                                printf("Tile choose %d\n", tilesTab[currentRound]);
                            }
                        }
                        // read score form player
                        else if(msg.code == SCORE){
                            addPlayerScore(i, msg.playerScore);
                            nbScoreSended++;
                        }
                    }
                }
            }


            printf("CALCULE DES SCORE ... \n");
            sortPlayerScore();

            // wait every child 
            for (int i = 0; i < nbPLayers; ++i)
            {
                swait(NULL);
            }

            // clean all pipe
            for (int i = 0; i < nbPLayers; ++i)
            {
                sclose(tabClients[i].pipefdParent[1]); 
                sclose(tabClients[i].pipefdChild[0]); 
            }

            //
            
        }

    
        ssigprocmask(SIG_UNBLOCK, &set, NULL);
    }

    sclose(sockfd); // close server socket
    clearSharedMemory(); // clear memory

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


    // close
    sclose(client->sockfd); // je ne comunique plus
    sclose(client->pipefdChild[1]); // je n'ecriverais plus
    sclose(client->pipefdParent[0]); // j'ecoute plus 

}

