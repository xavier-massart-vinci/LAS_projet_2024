/**
 * Authors: Debin Pierre-Alexandre
 *          Devos Thibaut
 *          Massart Xavier
 *
 *          Groupe 15
 */

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "messages.h"
#include "jeu.h"

#define BACKLOG 5
#define SERVER_IP "127.0.0.1" /* localhost */

/**
 * PRE: serverIP : a valid IP address
 *      serverPort: a valid port number
 * POST: on success, connects a client socket to serverIP:serverPort ;
 *       on failure, displays error cause and quits the program
 * RES: return socket file descriptor
 */
int initSocketClient(char *serverIP, int serverPort);

/**
 * PRE:  serverPort: a valid port number
 * POST: on success, binds a socket to 0.0.0.0:serverPort and listens to it ;
 *       on failure, displays error cause and quits the program
 * RES:  return socket file descriptor
 */
int initSocketServer(int port);

/**
 * PRE:  tabClients: a pointer to an array of Client objects
 *       nbPlayers: the number of players
 *       tilesChose: the chosen tiles
 * POST: Sends the chosen tiles to all clients
 */
void sendTile(Client *tabClients, int nbPlayers, int tilesChose);

#endif