#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include "orgy.h"


static struct sembuf buf;

void sem_up(int semid, int semnum);
void sem_down(int semid, int semnum);
int get_semaphores(int rank, int count);


#endif
