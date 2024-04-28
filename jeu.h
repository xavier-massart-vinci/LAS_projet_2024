#ifndef _JEU_H_
#define _JEU_H_

#include "messages.h"

#define PointsTab { 0, 1, 3, 5, 7, 9, 11, 15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 150, 300} 
#define TILES_TAB_SIZE 40
#define NB_ROUND 2

void createTiles(int* tilesTab);
int getRandomTile(int* tilesTab, int nbRemainingTiles);


#endif