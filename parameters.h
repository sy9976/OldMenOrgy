#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "orgy.h"
#include "communication.h"

typedef struct {
  int size;
  bool busy;
} Room;

typedef struct {
  int rank;
  int size;
  int semid;
  pthread_mutex_t mutex;
  Room *rooms; 
} threadParameters;

typedef struct {
  int clock;
  int room;
} dataToCompare;

bool check_parameteres_correctness(int s, int k, int m);
void initialize(int nRooms, int rank, int size, int nWomen, int nMen);
Room* generate_rooms(int s, int k, int m);
bool compare_priority(dataToCompare p1, dataToCompare p2);
bool compare_preferences(int rank1, int rank2);
int clock_priority(int clock1, int clock2);
int room_priority(int rank1, int rank2, int room, int nRooms);
int rank_priority(int rank1, int rank2);
int rank_to_priority(int rank, int room, int nRooms); //returns priority to room based on rank
bool compare_priority_sec1(int rank1, int rank2, int clock1, int clock2, int room, int nRooms);

#endif
