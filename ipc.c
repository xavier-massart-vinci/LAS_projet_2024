#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

#include "utils_v1.h"
#include "messages.h"
#include "network.h"
#include "jeu.h"




/*

int semInit(){
    return sem_create(SEM_KEY, 1, PERM, 0);
}

int shmInit(){
    return sshmget(SHEM_KEY, sizeof(TabPlayer), IPC_CREAT | IPC_EXCL | PERM);
}

TabPlayer* getTabPlayer(TabPlayer* tab){
    int shm_id = sshmget(SHEM_KEY, sizeof(TabPlayer), IPC_CREAT | IPC_EXCL | PERM);

    return (TabPlayer*) sshmat(shm_id);
}


void addPlayerScore(int playerIndex, int score){
    TabPlayer* tab = getTabPlayer(TabPlayer* tab);

    tab->tabPlayer[playerIndex].score = score;
}


int compare_scores(const void *a, const void *b) {
    const struct Player *playerA = (const struct Player *)a;
    const struct Player *playerB = (const struct Player *)b;
    return playerA->score - playerB->score;
}

void sortPlayerScore(){
    TabPlayer* tab = getTabPlayer(TabPlayer* tab);

    Player* tabPlayer =  tab->tabPlayer;
    
   qsort(tabPlayer, tab->nbrPlayer, sizeof(Player), compare_scores);
}

*/