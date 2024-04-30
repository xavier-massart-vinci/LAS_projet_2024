/**
 * Authors: Debin Pierre-Alexandre
 *          Devos Thibaut
 *          Massart Xavier
 *
 *          Groupe 15
 */

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

    // setsockopt -> to avoid Address Already in Use
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    sbind(port, sockfd);

    slisten(sockfd, BACKLOG);

    printf("Le serveur tourne sur le port : %i \n", port);

    return sockfd;
}

void sendTile(Client *tabClients, int nbPlayers, int tilesChose)
{
    Message msg;

    msg.code = TUILE_PIOCHEE;
    msg.tileTake = tilesChose;

    for (int j = 0; j < nbPlayers; ++j)
    {
        swrite(tabClients[j].pipefdParent[1], &msg, sizeof(msg));
    }
}