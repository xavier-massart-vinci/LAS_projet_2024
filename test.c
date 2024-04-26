#include <stdio.h>
#include <stdlib.h>


#include "utils_v1.h"
#include "messages.h"
#include "ipc.h"


int main(int argc, char const *argv[])
{
    /*
    semInit();
    shmInit();
    */



    TabPlayer* tab =  getTabPlayer();
  

    printf("Number of players : %d\n", tab->nbrPlayer);

    for (int i = 0; i < tab->nbrPlayer; ++i)
    {
        Player player = (tab->tabPlayer)[i];
        printf("Player %d Pseudo: %s Score: %d \n", i, player.pseudo, player.score);

    }





    /*





    TabPlayer* getTabPlayer();
    TabPlayer* readTabPlayer();
    // PRE : playerIndex : the index of score
    //       score : player score to add
    // POST: the score of player at index playerIndex has been added to shared memory
    void addPlayerScore(int playerIndex, int score);

    void sortPlayerScore();

    void clearPlayerScore();

*/
    return 0;
}