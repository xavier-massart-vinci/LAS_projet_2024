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

    printf("Le serveur tourne sur le port : %i \n", port);

    /* no listen error */
    return sockfd;
}

// send to very player the tuils
void sendTile(Client *tabClients, int nbPlayers, int tilesChose)
{
    Message msg;

    // prepare message
    msg.code = TUILE_PIOCHEE;
    msg.tileTake = tilesChose;

    // send to all player tile
    for (int j = 0; j < nbPlayers; ++j)
    {
        // send TUILE_PIOCHEE
        swrite(tabClients[j].pipefdParent[1], &msg, sizeof(msg));
    }
}