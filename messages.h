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






typedef struct Client
{
    int sockfd;
    int pipefdParent[2];
    int pipefdChild[2];
    int childPid;
} Client;

typedef struct Player
{
    char pseudo[MAX_PSEUDO];
    int score;
} Player;

typedef enum Code  {
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

typedef struct TabPlayer
{
    Player tabPlayer[MAX_PLAYERS];
    int nbrPlayer;
} TabPlayer;

typedef struct Message
{
    
    Code code;
    char pseudo[MAX_PSEUDO];
    int tileTake;
    int playerScore;
    TabPlayer tabPlayer;
} Message;




#endif