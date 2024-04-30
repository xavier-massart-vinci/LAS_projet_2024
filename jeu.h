/**
 * Authors: Debin Pierre-Alexandre, Devos Thibaut, Massart Xavier
 */

#ifndef _JEU_H_
#define _JEU_H_

#include "messages.h"

#define POINTS_TAB (int[20]){ 0, 1, 3, 5, 7, 9, 11, 15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 150, 300 }
#define TILES_TAB_SIZE 40
#define NB_ROUND 20

/**
 * PRE: tilesTab : a pointer to an array of integers.
 * POST: The `tilesTab` array is filled with random tiles.
 */
void createTiles(int *tilesTab);

/**
 * PRE: tilesTab : a pointer to an array of integers.
 *      nbRemainingTiles : the number of remaining tiles.
 * POST: A random tile is removed from `tilesTab`.
 * RES: Returns the value of the removed tile.
 */
int getRandomTile(int *tilesTab, int nbRemainingTiles);

/**
 * PRE:  tilesTab: a pointer to an array of integers
         file : the given file at server start
 * POST: generates the tiles for the game
 */
void setupTiles(int *tilesTab, FILE **file);

/**
 * PRE: tabPlayer : a pointer to a TabPlayer structure.
 * POST: The leaderboard is displayed in the console.
 */
void displayLeaderBoard(TabPlayer *tabPlayer);

/**
 * PRE: board : a pointer to an array of integers.
 * POST: The game board is displayed in the console.
 */
void displayBoard(int *board);

/**
 * PRE: board : a pointer to an array of integers.
 *      pos : the position to place the tile.
 *      tile : the tile to place.
 * POST: The tile is placed at the specified position on the board.
 */
void placeTile(int *board, int pos, int tile);

/**
 * PRE: board : a pointer to an array of integers.
 * RES: Returns the score calculated based on the state of the board.
 */
int calculateScore(int *board);

#endif