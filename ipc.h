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



//TODO







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


