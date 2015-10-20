#ifndef ORGY_H
#define ORGY_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <pthread.h>
#include <mpi.h>

//messages types
#define INIT 1
#define ROOM_REQ 2
#define ROOM_ACK 3
#define PEOPLE_REQ 4
#define PEOPLE_ACK 5
#define PEOPLE_NEW_VALUE 6

#define APPLY 0
#define RELEASE 1

#define CLOCK_DISPLAY 1
#define ACK_DISPLAY 1
#define REQ_DISPLAY 0

#endif
