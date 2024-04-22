#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#define SERVER_PORT 9502
#define SERVER_IP "127.0.0.1" /* localhost */
#define MAX_PSEUDO 255
#define MAX_PLAYERS 4
#define TIME_INSCRIPTION 15
#define SEM_KEY 123
#define PERM 0600
#define ROUNDS 20




typedef struct Client
{
    int sockfd;
    int pipefd;
    int pid;
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
    TUILE_PLACE,
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