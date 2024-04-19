#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#define SIZE_PSEUDO 255
#define MAX_JOUEUR 4

typedef struct 
{
    char pseudo[SIZE_PSEUDO];
    Code code;
} Message;


typedef enum  {
    INSCRIPTION_REQUETE,
    INSCRIPTION_OK,
    INSCRIPTION_KO,
    TUILE_PIOCHEE,
    TUILE_PLACE,
    SCORE,
    RANK
} Code;

typedef struct
{
    int sockfd;
    int pipefd;
    int pid;
} Client;

typedef struct 
{
    char pseudo[SIZE_PSEUDO];
    int score;
} Joueur;

typedef struct 
{
    Joueur tabjoueurs[MAX_JOUEUR];
    int nbrJoueur;
} TabJoueurs;

#endif