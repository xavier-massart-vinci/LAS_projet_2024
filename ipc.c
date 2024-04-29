/**
 * Authors: Debin Pierre-Alexandre, Devos Thibaut, Massart Xavier
 */

#include "ipc.h"

void createIPC()
{
    sem_create(SEM_KEY, 1, PERM, 0);
    sshmget(SHEM_KEY, sizeof(TabPlayer), IPC_CREAT | IPC_EXCL | PERM);
}

TabPlayer* getTabPlayer()
{
    int shm_id = sshmget(SHEM_KEY, sizeof(TabPlayer), PERM);
    return (TabPlayer *)sshmat(shm_id);
}

void addPlayer(char *pseudo)
{
    TabPlayer *tab = getTabPlayer();
    strcpy(tab->tabPlayer[tab->nbrPlayer].pseudo, pseudo);
    (tab->nbrPlayer)++;
    sshmdt(tab);
}

TabPlayer *readTabPlayer()
{
    int sem_id = sem_get(SEM_KEY, 1);

    // Blocked until the server has finished writing in the shared memory
    sem_down0(sem_id);
    TabPlayer *tab = (TabPlayer *)getTabPlayer();
    return tab;
}

void addPlayerScore(int playerIndex, int score)
{
    TabPlayer *tab = getTabPlayer();
    tab->tabPlayer[playerIndex].score = score;
    sshmdt(tab);
}

int compare_scores(const void *a, const void *b)
{
    const struct Player *playerA = (const struct Player *)a;
    const struct Player *playerB = (const struct Player *)b;
    if (playerA->score == playerB->score)
    {
        return 0;
    }
    if (playerA->score > playerB->score)
    {
        return -1;
    }

    return 1;
}

void sortPlayerScore()
{
    int sem_id = sem_get(SEM_KEY, 1);

    TabPlayer *tab = getTabPlayer();

    Player *tabPlayer = tab->tabPlayer;

    qsort(tabPlayer, tab->nbrPlayer, sizeof(Player), compare_scores);

    for (int i = 0; i < tab->nbrPlayer; ++i)
    {
        sem_up0(sem_id);
    }

    sshmdt(tab);
}

void resetSHM()
{
    TabPlayer *tab = getTabPlayer();

    for (int i = 0; i < tab->nbrPlayer; ++i)
    {
        tab->tabPlayer[i].pseudo[0] = '\0';
        tab->tabPlayer[i].score = 0;
    }

    tab->nbrPlayer = 0;

    sshmdt(tab);
}

void clearSHM()
{
    int sem_id = sem_get(SEM_KEY, 1);
    int shm_id = sshmget(SHEM_KEY, sizeof(TabPlayer), PERM);

    sem_delete(sem_id);
    sshmdelete(shm_id);
}
