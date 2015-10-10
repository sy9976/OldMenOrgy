#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include "orgy.h"

typedef struct {
  int rank;
  int size;
  int semid;
  pthread_mutex_t mutex;
} threadParameters;

static struct sembuf buf;

void sem_up(int semid, int semnum);
void sem_down(int semid, int semnum);
int get_semaphores(int rank, int count);


#endif
