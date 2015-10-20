#include "communication.h"


void end(){
  printf("---------------ZAKOŃCZENIE DZIAŁANIA PROGRAMU--------------\n");
  //free(rooms);
  MPI_Finalize();
  exit(0);
}
 
void send_to_all(int rank, int* data, int count, int tag, int size){
  int i = 0;
  for(i = 0; i < size; i++){
    if((i != rank) && (i != 0)){
      MPI_Send(data, count, MPI_INT, i, tag, MPI_COMM_WORLD);
      //printf("Wyslalem do %d\n", i);
    }
  }
}

void get_sizes_rooms(Room *rooms, int nRooms, MPI_Status* status){
  //MPI_Recv(rooms, nRooms, MPI_INT, 0, INIT, MPI_COMM_WORLD, status);
  int x;
  int i;
  for (i = 0; i < nRooms; i++){
    MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, INIT, MPI_COMM_WORLD, status);//any source na 0 i jakis define
    rooms[i].size = x;
  }
}
