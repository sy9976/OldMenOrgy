#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "orgy.h"
#include "semaphores.h"
#include "parameters.h"

void *reader(void *arg);
void end();
void send_to_all(int rank, int* data, int count, int tag, int size);


#endif
