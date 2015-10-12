#include "parameters.h"
#include "semaphores.h"

bool check_parameters_correctness(int s, int k, int m) 
{
  //printf("S:%d\tK:%d\tM:%d\n",s,k,m);
  if(s<=1) return false;
  if(ceil(((m+k)/s)<1)) return false;
  if((k<1)||(m<1)) return false;
	return true;
}

void initialize(int nRooms, int rank, int size, int nWomen, int nMen){
  printf("-----INICJALIZACJA-----\n");
  int i;
  int roomCapacity;
  signal(SIGINT, end);
  srand(time(NULL));
  Room *rooms;
  rooms = (Room*) malloc(nRooms * sizeof(*rooms));
  rooms = generate_rooms(nRooms,nWomen, nMen); //(int s, int k, int m)
  printf("-----NEW ROOMS SIZES----\n");
  for (i = 0; i < nRooms; i++){
  	printf("[%d] %d\n", i, rooms[i].size);
    send_to_all(rank, &(rooms[i].size), 1, INIT, size);
  }
  /*for (i = 0; i < nRooms; i++){
    roomCapacity = 2+rand()%10; //lepsza funkcja do ustalania rozmiaru sali (min 2 osoby, srednia +/- cos z modulo)
    send_to_all(rank, &roomCapacity, 1, INIT, size);
  }*/
  //send_to_all(rank, rooms, nRooms, INIT, size);
  printf("-----INICJALIZACJA ZAKOŃCZONA-----\n");
}  

Room* generate_rooms(int s, int k, int m)
{
  Room *rooms;
  rooms = (Room*) malloc(s * sizeof(*rooms));
  int i=0;
  printf("m: %d      k: %d     s: %d \n", k, m, s );
  int min_size = (int)ceil((((float)m+(float)k)/(float)s)*(((float)s+1)/(float)s));
  int max_size = (int)ceil((((float)m+(float)k)/(float)s)*(((float)s+3)/(float)s));
  int room_size;
  srand (time(NULL));
  printf("Min size of room: %d\nMax size of room: %d\n", min_size, max_size);
  for (i=0; i<=s-1; i++)
  {
    if ((int)max_size == (int)min_size)
      room_size = (int)min_size;
    else
    {
      room_size = rand() % ((int)max_size-((int)min_size)+1) + (int)min_size;
    }
    printf("New size for room %d : %d\n", i, room_size);
    rooms[i].size = room_size;
  }
  return rooms;
}


bool compare_preferences(int rank1, int rank2){//wywolywana w compare_priority
  return true;
}
 
bool compare_priority(dataToCompare p1, dataToCompare p2){
  return true;  
}

bool compare_priority_sec1(int rank1, int rank2, int clock1, int clock2, int room, int nRooms){
  int clockPriority = clock_priority(clock1, clock2);
  if (clockPriority == 1) return true;
  else if (clockPriority == 0) return false;
  else{
    int roomPriority = room_priority(rank1, rank2, room, nRooms);
    if (roomPriority == 1) return true;
    else if (roomPriority == 0) return false;
    else{
      if (rank_priority(rank1, rank2) == 1) return true;
      else return false;
    }
  }
}

int clock_priority(int clock1, int clock2){
  if (clock1 < clock2) return 0;
  else if (clock1 > clock2) return 1;
  else return -1;
}

int rank_to_priority(int rank, int room, int nRooms){
  if (rank <= room) return (room-rank);
  else {
    int prior = rank % nRooms;
    //printf("rank: %d prior: %d\n",rank, prior);
    if (prior < room) return room-prior;
    else return (nRooms - prior + room);
  }
}

int room_priority(int rank1, int rank2, int room, int nRooms){
  int r1 = rank_to_priority(rank1, room, nRooms);
  int r2 = rank_to_priority(rank2, room, nRooms);
  if (r1<r2) return 0;
  else if (r1>r2) return 1;
  else return -1;
}

int rank_priority(int rank1, int rank2){
  if (rank1 < rank2) return 0;
  else if (rank1 > rank2) return 1;
  else return -1;
}

preferencesData* get_preferences(int rank, int nRooms){
        preferencesData *rooms;
  rooms = (preferencesData*) malloc(nRooms * sizeof(*rooms));
        int prior = rank % nRooms;
        int i;
        for (i = 0; i < nRooms; i++){
                rooms[i].room = prior;
                rooms[i].sent = false;
                if (prior == nRooms - 1) prior = 0;
                else prior++;
                //printf("INSIDE %d", prior);
        }
        return rooms;
}


