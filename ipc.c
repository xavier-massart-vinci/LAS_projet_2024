#include "ipc.h"



// USE START


void semInit(){
    sem_create(SEM_KEY, 1, PERM, 0);
}

void shmInit(){
    sshmget(SHEM_KEY, sizeof(TabPlayer), IPC_CREAT | IPC_EXCL | PERM);
}


TabPlayer* getTabPlayer(){
    int shm_id = sshmget(SHEM_KEY, sizeof(TabPlayer),  PERM);

    return (TabPlayer*) sshmat(shm_id);
}


//TODO check if nbrPlayer == MAX_PLAYER
void addPlayer(char* pseudo){
    TabPlayer* tab = getTabPlayer();

    // write is name
    strcpy(tab->tabPlayer[tab->nbrPlayer].pseudo, pseudo);

 
    (tab->nbrPlayer)++;

    sshmdt(tab);
}


// USE END



// to detach
TabPlayer* readTabPlayer(){
    int sem_id = sem_get(SEM_KEY, 1);

    // blocked wait server
    sem_down0(sem_id);

    TabPlayer* tab = (TabPlayer*) getTabPlayer();

    return tab;
}


void addPlayerScore(int playerIndex, int score){
    TabPlayer* tab = getTabPlayer();

    tab->tabPlayer[playerIndex].score = score;

    sshmdt(tab);
}


int compare_scores(const void *a, const void *b) {
    const struct Player *playerA = (const struct Player *)a;
    const struct Player *playerB = (const struct Player *)b;
    return playerA->score - playerB->score;
}

void sortPlayerScore(){
    int sem_id = sem_get(SEM_KEY, 1);

    TabPlayer* tab = getTabPlayer();

    Player* tabPlayer =  tab->tabPlayer;
    
    qsort(tabPlayer, tab->nbrPlayer, sizeof(Player), compare_scores);

   // fils peuvent lire
   for (int i = 0; i < tab->nbrPlayer; ++i)
   {
        sem_up0(sem_id);
   }

   sshmdt(tab);
}


void cleanSHM(){
    TabPlayer* tab = getTabPlayer();

    for (int i = 0; i < tab->nbrPlayer; ++i)
    {
        tab->tabPlayer[i].pseudo[0] = '\0';// js si c'est legal
        tab->tabPlayer[i].score = 0;
    }

    tab->nbrPlayer = 0;

    sshmdt(tab);
}

void clearSharedMemory(){
    int sem_id = sem_get(SEM_KEY, 1);
    int shm_id = sshmget(SHEM_KEY, sizeof(TabPlayer),  PERM);

    sem_delete(sem_id);     
    sshmdelete(shm_id);
}
