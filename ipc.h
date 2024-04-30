/**
 * Authors: Debin Pierre-Alexandre
 *          Devos Thibaut
 *          Massart Xavier
 *
 *          Groupe 15
 */

#ifndef _IPC_H_
#define _IPC_H_

#include "messages.h"

#define SEM_KEY 123
#define SHEM_KEY 456

#define PERM 0600

/**
 * POST: A semaphore and a shared memory with their associated keys are created.
 */
void createIPC();

/**
 * PRE: pseudo : the pseudo of the player to add.
 * POST: The player has been added to the shared memory.
 */
void addPlayer(char *pseudo);

/**
 * RES: Returns a pointer to the TabPlayer object from the shared memory.
 */
TabPlayer *getTabPlayer();

/**
 * RES: Returns a pointer to the TabPlayer object from the shared memory.
 */
TabPlayer *readTabPlayer();

/**
 * PRE:  playerIndex : is the index of the player.
 *       score : the player score to add.
 * POST: The score of the player at index `playerIndex` has been added to the shared memory.
 */
void addPlayerScore(int playerIndex, int score);

/**
 * POST: The scores of the players in the shared memory are sorted.
 */
void sortPlayerScore();

/**
 * POST: The semaphore and the shared memory are destroyed.
 */
void resetSHM();

/**
 * POST: The shared memory is cleared.
 */
void clearSHM();

#endif