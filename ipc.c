#include "ipc.h"
#include "messages.h"
#include "utils_v1.h"

int semInit(){
    return sem_create(SEM_KEY, 1, PERM, 0);
}

