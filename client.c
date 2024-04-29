/**
 * Authors: Debin Pierre-Alexandre, Devos Thibaut, Massart Xavier
 */

#include "ipc.h"
#include "jeu.h"
#include "network.h"

int main(int argc, char const *argv[])
{
    int sockfd;
    Message msg;

    /* retrieve player name */
    printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
    printf("Pour participer entrez votre nom :\n");
    char* pseudo = readLine();
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

    if (msg.code == CANCEL_GAME)
    {
        printf("Partie annulée par manque de joueurs");
        sclose(sockfd);
        return 0;
    }

    int board[NB_ROUND] = {0};
    displayBoard(board);

    for (int i = 0; i < NB_ROUND; ++i)
    {
        Message msg;

        // wait TUILE_PIOCHEE
        sread(sockfd, &msg, sizeof(msg));

        printf("\033[1;36mTuile %d piochée, veuillez choisir une case\033[0m\n", msg.tileTake);
        char *ligneRead = readLine();
        int posPlayed = atoi(ligneRead);

        // send TUILE_PLACEE
        placeTile(board, posPlayed, msg.tileTake);
        msg.code = TUILE_PLACEE;
        swrite(sockfd, &msg, sizeof(msg));

        displayBoard(board);
        free(ligneRead);
    }

    // send SCORE
    msg.code = SCORE;
    msg.playerScore = calculateScore(board);
    swrite(sockfd, &msg, sizeof(msg));

    // wait RANK
    sread(sockfd, &msg, sizeof(msg));

    TabPlayer tab = msg.tabPlayer;
    displayLeaderBoard(&tab);

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