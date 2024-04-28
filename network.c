#include "network.h"


int initSocketClient(char *serverIP, int serverPort)
{
    int sockfd = ssocket();
    sconnect(serverIP, serverPort, sockfd);
    return sockfd;
}


int initSocketServer(int port)
{
    int sockfd = ssocket();

    /* no socket error */

    // setsockopt -> to avoid Address Already in Use
    // to do before bind !
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    sbind(port, sockfd);

    /* no bind error */
    slisten(sockfd, BACKLOG);

    /* no listen error */
    return sockfd;
}


// send to very player the tuils
void selectAndSendTile(Client* tabClients, int nbPlayers, int* tilesTab, int currentRound)
{
    Message msg;

    // prepare message 
    msg.code = TUILE_PIOCHEE;
    msg.tileTake = getRandomTile(tilesTab, TILES_TAB_SIZE - currentRound);

    // send to all player tile
    for (int j = 0; j < nbPlayers; ++j)
    {
        // send TUILE_PIOCHEE
        swrite(tabClients[j].pipefdParent[1], &msg, sizeof(msg));
    }
}