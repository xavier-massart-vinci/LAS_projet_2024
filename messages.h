/**
 * Authors: Debin Pierre-Alexandre, Devos Thibaut, Massart Xavier
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>

#include "utils_v1.h"

#define MAX_PSEUDO 255
#define MAX_PLAYERS 4
#define TIME_INSCRIPTION 15

/**
 * Structure representing a client.
 * sockfd: the socket file descriptor
 * pipefdParent: the pipe file descriptor for the parent process
 * pipefdChild: the pipe file descriptor for the child process
 * childPid: the process ID of the child process
 */
typedef struct Client
{
    int sockfd;
    int pipefdParent[2];
    int pipefdChild[2];
    int childPid;
} Client;

/**
 * Structure representing a player.
 * pseudo: the player's pseudonym
 * score: the player's score
 */
typedef struct Player
{
    char pseudo[MAX_PSEUDO];
    int score;
} Player;

/**
 * Enumeration representing various message codes.
 */
typedef enum Code
{
    INSCRIPTION_REQUEST,
    INSCRIPTION_OK,
    INSCRIPTION_KO,
    TUILE_PIOCHEE,
    TUILE_PLACEE,
    CANCEL_GAME,
    START_GAME,
    SCORE,
    RANK
} Code;

/**
 * Structure representing a table of players.
 * tabPlayer: an array of Player structures
 * nbrPlayer: the number of players
 */
typedef struct TabPlayer
{
    Player tabPlayer[MAX_PLAYERS];
    int nbrPlayer;
} TabPlayer;

/**
 * Structure representing a message.
 * code: the message code
 * pseudo: the pseudonym of the player
 * tileTake: the tile taken by the player
 * playerScore: the score of the player
 * tabPlayer: a TabPlayer structure
 */
typedef struct Message
{
    Code code;
    char pseudo[MAX_PSEUDO];
    int tileTake;
    int playerScore;
    TabPlayer tabPlayer;
} Message;

#endif