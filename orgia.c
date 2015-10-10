#include <mpi.h>
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
#include "parameters.h"
#include "orgy.h"
 
#define INIT 1
#define ROOM_REQ 2
 
 
typedef struct {
  int rank;
  int size;
  int semid;
  pthread_mutex_t mutex;
} threadParameters;
 
typedef struct {
  int clock;
  int room;
} dataToCompare;
 
static struct sembuf buf;
 
bool compare_preferences(int rank1, int rank2){//wywolywana w compare_priority
  return true;
}
 
bool compare_priority(dataToCompare p1, dataToCompare p2){
  return true;  
}
 
void *reader(void *arg){
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
 
void sem_up(int semid, int semnum){
  buf.sem_num = semnum;
  buf.sem_op = 1;
  buf.sem_flg = 0;
  if (semop(semid, &buf, 1) == -1){
    printf("Błąd podnoszenia semafora\n");
    exit(1);
  }
}
 
void sem_down(int semid, int semnum){
  buf.sem_num = semnum;
  buf.sem_op = -1;
  buf.sem_flg = 0;
  if (semop(semid, &buf, 1) == -1){
    printf("Błąd opuszczania semafora\n");
    exit(1);
  }
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
 
void get_sizes_rooms(Room *rooms, int nRooms, MPI_Status* status){
  int x;
  int i;
  for (i = 0; i < nRooms; i++){
    MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, INIT, MPI_COMM_WORLD, status);//any source na 0 i jakis define
    rooms[i].size = x;
  }
}
 
int get_semaphores(int rank, int count){
  int semid = semget(40000+rank, count, IPC_CREAT|0600);
  if (semid == -1){
    printf("[ID: %d] Błąd tworzenia semaforów.\n", rank);
    MPI_Finalize();
    exit(1);
  }
  if (semctl(semid, 0, SETVAL, (int)0) == -1){
    printf("[ID: %d] Błąd nadania wartości semaforowi.\n", rank);
    MPI_Finalize();
    exit(1);
  }
  if (count > 1) {
    if (semctl(semid, 1, SETVAL, (int)1) == -1) {
      printf("[ID: %d] Błąd nadania wartości semaforowi.\n", rank);
      MPI_Finalize();
      exit(1);
    }
  }
  return semid;
}
 
int main (int argc, char **argv){
  MPI_Init(&argc, &argv);
  MPI_Status status;
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  //////////////////////ODCZYT PARAMETRÓW
  int nRooms = atoi(argv[1]);
  int nWomen = atoi(argv[2]);//string to int
  int nMen = atoi(argv[3]);
  
  if (argc < 4)
  {
    printf("Nie podano wszystkich parametrow!\n");
  }
  else if(!check_parameters_correctness(nRooms, nWomen, nMen)) //int s, int k, int m
  {
    printf("Podane parametry nie spełniają podstawowych założeń!\nK+M/S >=1\nK>=1   M>=1   S>=1\n");
  }
  else{
    
    /////////////////////////INICJATOR
    if (rank == 0){
      printf("Liczba staruszek: %d, liczba staruszków: %d, liczba sal: %d\n", nWomen, nMen, nRooms);
      //printf("Glowny, moj nr to:%d \n", rank);
      initialize(nRooms, rank, size, nWomen, nMen);
    }
 
    /////////////////////////NORMALNY PROCES
    else{
      //printf("Moj nr to: %d \n", rank);
      int clock = 0;
      Room *rooms;
      rooms = (Room*) malloc(nRooms * sizeof(*rooms));
      get_sizes_rooms(rooms, nRooms, &status);
 
      printf("[ID: %d]Pojemności sal: ", rank);
      int i;
      for (i = 0; i < nRooms; i++){  
        printf("%d ", rooms[i].size);
      }
      printf("\n");
 
 
      /*fork - podzial na proces czytajacy i proces wykonujacy dostepy do sekcji
       *Dostep do pierwszej sekcji krytycznej - wybor sali (kazdy ma swoje preferencje co do sali ustalone
       *przez funkcje %rank. algorytm agrawala (kazdy proces ma swoj zegar)
       *
       *
       *
       *TAGI: init, req_room, req_people, ans_room, ans_people
       *init: tablica pojemnosci sal
       * req_room: zegar, nr sali
       * req_people: zegar
       * ans_room: zegar, zgoda
       * ans_people: zegar, zgoda
       *
       *
       *
       *animator:
       *  zegar,
       *  lista typkow ktorym musimy dac odpowiedz
       *  ile razy staramy sie o maksymalne wypelnienie sali zanim ropoczniemy sex
       *  liczba otrzymanych zgod
       *  preferencje co do sal(funkcja od rank)
       *
       *sekcje:
       *  1 dla k+m
       *  s sekcji sal: kolejnosc przydzielania -zgody, zegar, preferencje(funkcja), rank(id)
       *  
       *
       */
      int semid = get_semaphores(rank, 1);
      pthread_t readerThread;
      threadParameters parameters;
      parameters.rank = rank;
      parameters.size = size;
      parameters.semid = semid;
      pthread_mutex_init(&(parameters.mutex), NULL);
      if (pthread_create(&readerThread, NULL, reader, &parameters)){
        printf("Błąd tworzenia wątka.\n");
        abort();
      }
      int data[2];
      data[0] = clock;
      data[1] = 2;
      send_to_all(rank, data, sizeof(data), ROOM_REQ, size);
      //send_request();
      printf("[ID: %d]Zglosiłem żądanie i czekam...\n", rank);
      sem_down(semid, 0);
     
 
      /*if (fork() == 0){
 
        printf("[ID: %d]Czekam 1\n", rank);
        sem_down(semid, 1);
        printf("[ID: %d]Jestem w sekcji\n", rank);
        sem_up(semid, 0);
        printf("[ID: %d]Przeszedlem\n", rank);
        MPI_Finalize();
        exit(0);
        }
        else{
        printf("[ID: %d]Czekam 2\n", rank);
        sem_down(semid, 0);
        printf("[ID: %d]Jestem w sekcji\n", rank);
        sem_up(semid, 1);
        printf("[ID: %d]Przeszedlem\n", rank);
        MPI_Finalize();
        exit(0);
        }*/
      printf("[ID: %d]Wyszedlem\n", rank);
 
      //free(rooms);
    }
  }
  //printf("[ID: %d]KONIEC\n", rank);
  MPI_Finalize();
  //printf("[ID: %d]KONIEC 2\n", rank);
  exit(0);
}
