#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "orgy.h"
#include "communication.h"

typedef struct {
  int size;
  bool busy;
} Room;

typedef struct {
  int clock;
  int room;
} dataToCompare;

bool check_parameteres_correctness(int s, int k, int m);
void initialize(int nRooms, int rank, int size, int nWomen, int nMen);
Room* generate_rooms(int s, int k, int m);
bool compare_priority(dataToCompare p1, dataToCompare p2);
bool compare_preferences(int rank1, int rank2);

#endif
