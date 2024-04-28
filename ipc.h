#ifndef _IPC_H_
#define _IPC_H_

#include "messages.h"

#define SEM_KEY 123
#define SHEM_KEY 456

#define PERM 0600


//POST: creates a semaphore associated with key,
void semInit();

void shmInit();


void addPlayer(char* pseudo);

TabPlayer* getTabPlayer();
TabPlayer* readTabPlayer();
// PRE : playerIndex : the index of score
//       score : player score to add
// POST: the score of player at index playerIndex has been added to shared memory
void addPlayerScore(int playerIndex, int score);

void sortPlayerScore();

void clearPlayerScore();
void clearSharedMemory();

#endif


