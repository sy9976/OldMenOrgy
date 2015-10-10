#include "parameters.h"
#include "semaphores.h"

bool check_parameters_correctness(int s, int k, int m) 
{
  printf("S:%d\tK:%d\tM:%d\n",s,k,m);
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
  }
  for (i = 0; i < nRooms; i++){
    roomCapacity = 2+rand()%10; //lepsza funkcja do ustalania rozmiaru sali (min 2 osoby, srednia +/- cos z modulo)
    send_to_all(rank, &roomCapacity, 1, INIT, size);
  }
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


