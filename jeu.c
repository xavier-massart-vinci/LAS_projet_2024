#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>

#include "utils_v1.h"
#include "network.h"
#include "jeu.h"
#include "messages.h"


void createTiles(int* tilesTab) 
{

    int k = 0;
    for (int i = 1; i <= 10; ++i)
    {
        tilesTab[k] = i;
        k++;
    }

    
    for (int j = 11; j <= 19; ++j)
    {
        for (int i = 0; i < 2; ++i)
        {
            tilesTab[k] = j;
            k++;
        }
    }

    for (int i = 20; i <= 31; ++i)
    {
        tilesTab[k] = i;
        k++;
    }

}

int getRandomTile(int* tilesTab, int nbRemainingTiles)
{
    srand(time(NULL));

    /* Get a random number on the remaining tiles */
    int randomNumber = rand() % nbRemainingTiles;
    int remainginTilePos = 0;

    /* If the tiles is not set to 0, add 1 remainginTilePos */
    for (int i = 0; i < TILES_TAB_SIZE; i++) {
        if (tilesTab[i] != 0) {
            if(randomNumber == remainginTilePos){
                int ret = tilesTab[i];
                tilesTab[i] = 0;
                return ret;
            }
            remainginTilePos++;
        }
    }
    return -1;
}
