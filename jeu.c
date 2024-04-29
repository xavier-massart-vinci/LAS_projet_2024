/**
 * Authors: Debin Pierre-Alexandre, Devos Thibaut, Massart Xavier
 */

#include "jeu.h"

void createTiles(int *tilesTab)
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

int getRandomTile(int *tilesTab, int nbRemainingTiles)
{
    srand(time(NULL));

    /* Get a random number on the remaining tiles */
    int randomNumber = rand() % nbRemainingTiles;
    int remainginTilePos = 0;

    /* If the tiles is not set to 0, add 1 remainginTilePos */
    for (int i = 0; i < TILES_TAB_SIZE; i++)
    {
        if (tilesTab[i] != 0)
        {
            if (randomNumber == remainginTilePos)
            {
                int ret = tilesTab[i];
                tilesTab[i] = 0;
                return ret;
            }
            remainginTilePos++;
        }
    }
    return -1;
}

void displayLeaderBoard(TabPlayer *tabPlayer)
{

    for (int i = 0; i < tabPlayer->nbrPlayer; ++i)
    {
        Player player = (tabPlayer->tabPlayer)[i];
        printf("Pseudo: %s Score: %d \n", player.pseudo, player.score);
    }
}

void displayBoard(int *board)
{

    // Display the round number
    printf("|");
    for (int i = 0; i < NB_ROUND; ++i)
    {
        printf("%3d |", i + 1);
    }
    printf("\n");

    // Display horizontal border
    for (int i = 0; i < NB_ROUND; ++i)
    {
        printf("-----");
    }
    printf("-\n");

    // Display the number played on the board
    printf("|");
    for (int i = 0; i < NB_ROUND; ++i)
    {
        if (board[i] == 0)
            printf(" __ |");
        else
            printf("%3d |", board[i]);
    }
    printf("\n");
}

void placeTile(int *board, int pos, int tile)
{
    pos = pos - 1;

    if (board[pos] == 0)
    {
        board[pos] = tile;
    }
    else if (pos + 1 == NB_ROUND)
    {
        placeTile(board, 1, tile);
    }
    else
    {
        placeTile(board, pos + 2, tile);
    }
}

int calculateScore(int *board)
{
    int score = 0;
    int suite = 0;

    for (int i = 1; i < NB_ROUND; i++)
    {
        if (board[i - 1] > board[i] || (board[i - 1] == 31 && board[i - 2] > board[i]))
        {
            // calculer score

            score = score + POINTS_TAB[suite];
            suite = 0;
            continue;
        }
        suite++;
    }
    score = score + POINTS_TAB[suite];
    return score;
}
