#include "communication.h"

void *reader(void *arg){ //monitor process
  threadParameters *parameters = arg;
  MPI_Status status;
  int size = parameters->size;
  int rank = parameters->rank;
  //int ackArray[size]; malloc, inicjalizacja
  int i = 0;
  printf("[ID: %d] Rozpoczęcie wątka czytającego.\n", rank);
  while(true) {
    int data[3];
    int x;
    MPI_Recv(data, sizeof(data), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == ROOM_REQ){
      printf("[ID: %d][ROOM_REQ: %d] Odebrałem żądanie dostępu do sali: %d.\n", rank, status.MPI_SOURCE, data[1]);
      //compare_priotiry();
      //if mam gorszy priorytet wyslij zgode ACK_ROOM(zegar moj, zegar req, sala)
      //jezeli mam lepszy buforuje
    }
    i++;
    if(i == size-2){
      printf("[ID: %d] Zebrałem zgody.\n", rank);
      break;
    }
  }
  printf("[ID: %d] Zakończenie wątka czytającego.\n", rank);
  MPI_Finalize();
  exit(0);
}
 
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
