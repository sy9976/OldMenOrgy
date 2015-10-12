#include "parameters.h"
#include "semaphores.h"
#include "communication.h"
#include "orgy.h"

int sectionClock = 0;
bool ack1Collected;
bool ack2Collected;

int main (int argc, char **argv){
  MPI_Init(&argc, &argv);
  MPI_Status status;
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  //reading parameters
  int nRooms = atoi(argv[1]);
  int nWomen = atoi(argv[2]);
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
      //int clock = 0;
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
      bool ack1Collected = false;
      int semid = get_semaphores(rank, 1);
      pthread_t readerThread;
      threadParameters parameters;
      parameters.rank = rank;
      parameters.size = size;
      parameters.nRooms = nRooms;
      //parameters.clock = &clock;
      parameters.semid = semid;
      parameters.rooms = rooms;
      //parameters.ack1Collected = &ack1Collected;
      pthread_mutex_init(&(parameters.mutex), NULL);
      if (pthread_create(&readerThread, NULL, reader, &parameters)){
        printf("Błąd tworzenia wątka.\n");
        abort();
      }
      preferencesData *preferencesArray;// = funckjaSzymona();
      while(true){
        int j;
        bool roomAchieved = false;
        for (i = 0; i < 3; i++){
          for (j = 0; j < nRooms; j++){
            if (!preferencesArray[j].sent){
              int data[2];
              data[0] = sectionClock;
              data[1] = 2;
              send_to_all(rank, data, sizeof(data), ROOM_REQ, size);
            }
            sleep(1000);
            if (ack1Collected){
              printf("[ID: %d][SKECJA 1]Uzyskałem dostęp do sali %d.\n", rank, preferencesArray[j].room);
              roomAchieved = true;
              break;
            }
          }
          if (roomAchieved){
            break;
          }
        }
        if (!roomAchieved){
          printf("[ID: %d][SEKCJA 1]Nie udało się uzyskać dostępu do żadnej sali.\n", rank);
          sleep(5000);
          continue;
        }
        ////////////////////////////////SEKCJA 1
        //
        //
        //
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
      }
      //free(rooms);
    }
  }
  //printf("[ID: %d]KONIEC\n", rank);
  MPI_Finalize();
  //printf("[ID: %d]KONIEC 2\n", rank);
  exit(0);
}
