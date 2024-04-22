#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>


#include "utils_v1.h"
#include "network.h"
#include "jeu.h"
#include "messages.h"


int main(int argc, char const *argv[])
{
    char pseudo[MAX_PSEUDO];
    int sockfd;
    int ret;

    Message msg;

    /* retrieve player name */
    printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
    printf("Pour participer entrez votre nom :\n");
    ret = sread(0, pseudo, MAX_PSEUDO);
    checkNeg(ret, "read client error");
    pseudo[ret - 1] = '\0';
    strcpy(msg.pseudo, pseudo);
    msg.code = INSCRIPTION_REQUEST;

    sockfd = initSocketClient(SERVER_IP, SERVER_PORT);

    swrite(sockfd, &msg, sizeof(msg));

    /* wait server response */
    sread(sockfd, &msg, sizeof(msg));

    switch (msg.code)
    {
    case INSCRIPTION_OK:
        printf("Réponse du serveur : Inscription acceptée\n");
        break;
    case INSCRIPTION_KO:
        printf("Réponse du serveur : Inscription refusée\n");
        sclose(sockfd);
        exit(0);
    default:
        printf("Réponse du serveur non prévue %d\n", msg.code);
        break;
    }

    /* wait start of game or cancel */
    sread(sockfd, &msg, sizeof(msg));

    /* end */
    sclose(sockfd);
    return 0;

    /* 
    CONNECTION
    
    demande pseudo
    envoie pseudo serveur
    reçoit réponse
    */ 

    /*
    GAME

    -> repeat 20 times (use ROUNDS)
    
    attend msg qui dit qu'une tuile est tirée + la tuile
    affiche la tuile reçu
    choix de l'emplacement de la tuile
    envoie la tuile au server
    affiche la nouvelle grille du joueur
    */ 

    /*
    END GAME

    calcul du score
    envoie du score au server
    attente du leaderboard (RANK + tab)
    affiche le leaderboard
    */

    /*
    END
    
    Ferme toutes les connections
    se termine
    */
}