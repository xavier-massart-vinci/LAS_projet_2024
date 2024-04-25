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
#include "messages.h"

#define PointsTab { 0, 1, 3, 5, 7, 9, 11, 15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 150, 300} 
#define TILES_TAB_SIZE 40
#define NB_ROUND 20

void createTiles(int* tilesTab);
int getRandomTile(int* tilesTab, int nbRemainingTiles);