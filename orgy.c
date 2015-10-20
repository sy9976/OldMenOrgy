#include "parameters.h"
#include "semaphores.h"
#include "communication.h"
#include "orgy.h"

int sectionClock = 0;
bool ack1Collected = false;
bool ack2Collected = false;
bool inSec1 = false;
bool inSec2 = false;
bufferData *bufferSec1;
int *bufferSec2;
int epochNo = 0;
int myRoom = -1;
int nWomen = 0;
int nMen = 0;
int peopleReqNo = -1;
bool sec2Apply = false;
int ack1Counter = 0;
int ack2Counter = 0;
int myPeopleStatus = APPLY;

void *reader(void *arg){ //monitor process
  threadParameters *parameters = arg;
  MPI_Status status;
  int size = parameters->size;
  int rank = parameters->rank;
  int nRooms = parameters->nRooms;
  int semid_sec1 = parameters->semid_sec1;
  int semid_sec2 = parameters->semid_sec2;
  int i = 0;
  //int ack1Counter = 0;
  if(CLOCK_DISPLAY) printf("%ld ",clock());
  printf("[ID: %d] Rozpoczęcie wątka czytającego.\n", rank);
  while(true) {
    int data[3];
    MPI_Recv(data, sizeof(data), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == ROOM_REQ){
    	if(REQ_DISPLAY) {
    		if(CLOCK_DISPLAY) printf("%ld ",clock());
      	printf("[CLOCK: %d][ID: %d][ROOM_REQ: %d] Odebrałem żądanie dostępu do sali: %d.\n", sectionClock, rank, status.MPI_SOURCE, data[1]);
      	}
      sem_down(semid_sec1, 0);
      if (data[1] == myRoom){
        if (!(ack1Collected) && (compare_priority_sec1(rank, status.MPI_SOURCE, sectionClock, data[0], data[1], nRooms))){
          MPI_Send(data, sizeof(data), MPI_INT, status.MPI_SOURCE, ROOM_ACK, MPI_COMM_WORLD);
        }
        else{
          bufferData tmpData;
          tmpData.clock = data[0];
          tmpData.epochNo = data[2];
          bufferSec1[status.MPI_SOURCE] = tmpData;
        }
      }
      else{
          MPI_Send(data, sizeof(data), MPI_INT, status.MPI_SOURCE, ROOM_ACK, MPI_COMM_WORLD); 
      }
      sem_up(semid_sec1, 0);

    }
    else if (status.MPI_TAG == ROOM_ACK){
      //printf("[CLOCK: %d][ID: %d][ROOM_ACK: %d] Odebrałem zgodę do sali: %d.\n", rank, status.MPI_SOURCE, data[1]);
      if ((sectionClock == data[0]) && (myRoom == data[1])){
        if (epochNo == data[2]){
        	if(ACK_DISPLAY) {
        		if(CLOCK_DISPLAY) printf("%ld ",clock());
        		printf("[CLOCK: %d][ID: %d][ROOM_ACK: %d][SALA: %d][EPOCHNO:%d][COUNTER: %d]\n", sectionClock, rank, status.MPI_SOURCE, data[1], epochNo, ack1Counter);
        		}
          ack1Counter++;
          if (ack1Counter == size - 2){
            if(CLOCK_DISPLAY) printf("%ld ",clock());
            printf("[CLOCK: %d][ID: %d][ROOM_ACK: %d] Zebrałem zgody do sali: %d.\n", sectionClock, rank, status.MPI_SOURCE, data[1]);
            ack1Collected = true;
            ack1Counter = 0;
          }
        }
      }
    }
    else if (status.MPI_TAG == PEOPLE_REQ){
      //printf("[CLOCK: %d][ID: %d][PEOPLE_REQ: %d] Odebrałem żądanie dostępu do ludzikow.\n", sectionClock, rank, status.MPI_SOURCE);
      if (ack2Collected){ 
      	bufferSec2[status.MPI_SOURCE] = data[0];
      	}
      else if (sec2Apply){
        if (data[1] == myPeopleStatus){
          sem_down(semid_sec2, 0);

          if ((!ack2Collected) && (compare_priority_sec2(rank, status.MPI_SOURCE, sectionClock, data[0]))){
            MPI_Send(data, sizeof(data), MPI_INT, status.MPI_SOURCE, PEOPLE_ACK, MPI_COMM_WORLD);
            //printf("[CLOCK: %d][ID: %d][PEOPLE_REQ: %d] Jestem gorszy i wysylam zgode.\n", sectionClock, rank, status.MPI_SOURCE);
          }
          else{
            //printf("[CLOCK: %d][ID: %d][PEOPLE_REQ: %d] Jestem lepszy i buforuje.\n", sectionClock, rank, status.MPI_SOURCE);
            bufferSec2[status.MPI_SOURCE] = data[0];
          }
          sem_up(semid_sec2, 0);
        }
        else if (myPeopleStatus == APPLY){
            MPI_Send(data, sizeof(data), MPI_INT, status.MPI_SOURCE, PEOPLE_ACK, MPI_COMM_WORLD);
        }
        else{
            bufferSec2[status.MPI_SOURCE] = data[0];
        }

      }
      else{
        MPI_Send(data, sizeof(data), MPI_INT, status.MPI_SOURCE, PEOPLE_ACK, MPI_COMM_WORLD);
      }
    }
    else if (status.MPI_TAG == PEOPLE_ACK){
      //printf("[CLOCK: %d][ID: %d][PEOPLE_ACK: %d] Odebrałem zgode na dostep do ludzikow.\n", sectionClock, rank, status.MPI_SOURCE);
      if (sectionClock == data[0]){
          ack2Counter++;
          //int tmp = peopleReqNo * (size - 2);
          //printf("[CLOCK: %d][ID: %d][PEOPLE_ACK: %d] Zebrałem %d zgod do ludzikow, potrzebuje %d\n", sectionClock, rank, status.MPI_SOURCE, ack2Counter, tmp);
          if (ack2Counter == peopleReqNo * (size - 2)){
            if(CLOCK_DISPLAY) printf("%ld ",clock());
            printf("[CLOCK: %d][ID: %d][PEOPLE_ACK: %d] Zebrałem zgody do ludzikow.\n", sectionClock, rank, status.MPI_SOURCE);
            ack2Collected = true;
            ack2Counter = 0;
          }
      }
    }
    else if (status.MPI_TAG == PEOPLE_NEW_VALUE){//nie potrzeba semafora, bo nowa liczba ludzikow przyjdzie przed ostatnia zgoda
      nWomen = data[0];
      nMen = data[1];
    }
    else{
      //syf
    }
  }
  if(CLOCK_DISPLAY) printf("%ld ",clock());
  printf("[CLOCK: %d][ID: %d] Zakończenie wątka czytającego.\n", sectionClock, rank);
  //MPI_Finalize();
  //exit(0);
}


int main (int argc, char **argv){
  MPI_Init(&argc, &argv);
  MPI_Status status;
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  //reading parameters
  int nRooms = atoi(argv[1]);
  nWomen = atoi(argv[2]);
  nMen = atoi(argv[3]);
  bufferSec1 = (bufferData*) malloc((size-1) * sizeof(*bufferSec1));
  bufferSec2 = (int*) malloc((size-1) * sizeof(*bufferSec1));
  int i;
  for (i = 0; i < size - 1; i++){
    bufferSec1[i].clock = -1;
    bufferSec2[i] = -1;
  }

  if (argc < 4)
  {
    if(CLOCK_DISPLAY) printf("%ld ",clock());
    printf("Nie podano wszystkich parametrow!\n");
  }
  else if(!check_parameters_correctness(nRooms, nWomen, nMen)) //int s, int k, int m
  {
    if(CLOCK_DISPLAY) printf("%ld ",clock());
    printf("Podane parametry nie spełniają podstawowych założeń!\nK+M/S >=1\nK>=1   M>=1   S>=1\n");
  }
  else{

    /////////////////////////INICJATOR
    if (rank == 0){
      if(CLOCK_DISPLAY) printf("%ld ",clock());
      printf("Liczba staruszek: %d, liczba staruszków: %d, liczba sal: %d\n", nWomen, nMen, nRooms);
      initialize(nRooms, rank, size, nWomen, nMen);
    }

    /////////////////////////NORMALNY PROCES
    else{
      Room *rooms;
      rooms = (Room*) malloc(nRooms * sizeof(*rooms));
      get_sizes_rooms(rooms, nRooms, &status);
			if(CLOCK_DISPLAY) printf("%ld ",clock());
      printf("[ID: %d]Pojemności sal: ", rank);
      int i;
      for (i = 0; i < nRooms; i++){  
        printf("%d ", rooms[i].size);
      }
      printf("\n");
      int semid_sec1 = get_semaphores(rank, 1);
      int semid_sec2 = get_semaphores(size+rank, 1);
      pthread_t readerThread;
      threadParameters parameters;
      parameters.rank = rank;
      parameters.size = size;
      parameters.nRooms = nRooms;
      parameters.semid_sec1 = semid_sec1;
      parameters.semid_sec2 = semid_sec2;
      parameters.rooms = rooms;
      //pthread_mutex_init(&(parameters.mutex), NULL);
      if (pthread_create(&readerThread, NULL, reader, &parameters)){
        if(CLOCK_DISPLAY) printf("%ld ",clock());
        printf("Błąd tworzenia wątka.\n");
        abort();
      }
      preferencesData *preferencesArray = get_preferences(rank, nRooms);
      while(true){
        int j;
        bool roomAchieved = false;
        bool peopleAchieved = false;
        int myMen = 0 ;
        int myWomen = 0;
        ////////////////////////////////WSTEP DO SEKCJI 1
        for (i = 0; i < 3; i++){
          for (j = 0; j < nRooms; j++){
            //ustawienie kolejnego pokoju
            sem_down(semid_sec1, 0);
            int k;
            for (k = 0; k < (size-1); k++){
              if (bufferSec1[k].clock > -1){
                int data[3];
                data[0] = bufferSec1[k].clock;
                data[1] = myRoom;
                data[2] = bufferSec1[k].epochNo;
                MPI_Send(data, sizeof(data), MPI_INT, k, ROOM_ACK, MPI_COMM_WORLD); //================k
                bufferSec1[k].clock = -1; //czyszczenie bufora;
                //printf("[ID: %d][SKEJCA 1] %d.\n", rank, myRoom);
              }
            }
            myRoom = preferencesArray[j].room;
            //printf("ZERUJE");
            ack1Counter = 0;
            sem_up(semid_sec1, 0);

            //wyslanie requesta
            if (!preferencesArray[j].sent){
              int data[3];
              data[0] = sectionClock;
              data[1] = preferencesArray[j].room;
              data[2] = epochNo;
              send_to_all(rank, data, sizeof(data), ROOM_REQ, size);
              if(CLOCK_DISPLAY) printf("%ld ",clock());
              printf("[CLOCK: %d][ID: %d][SKECJA 1]Żądam dostępu do sali %d.\n", sectionClock, rank, myRoom);
            }

            //czekanie na zgody
            sleep(5);
            if (ack1Collected){
            	if(CLOCK_DISPLAY) printf("%ld ",clock());
              printf("[CLOCK: %d][ID: %d][SKECJA 1]Uzyskałem dostęp do sali %d.\n", sectionClock, rank, preferencesArray[j].room);
              roomAchieved = true;
              sectionClock++;//1 punkt osiagniety
              //ack2Counter = 0;
              ack1Counter = 0;
              break;
            }
            //printf("[ID: %d][SKECJA 1]TEST1 %d.\n", rank, preferencesArray[j].room);

          }
          if (roomAchieved){
            break;
          }
          epochNo++;
        }
        
        if (!roomAchieved){
          if(CLOCK_DISPLAY) printf("%ld ",clock());
          printf("[CLOCK: %d][ID: %d][SEKCJA 1]Nie udało się uzyskać dostępu do żadnej sali.\n", sectionClock, rank);
          sleep(5);
          continue;
        }
        ////////////////////////////////SEKCJA 1

        ////////////////////////////////WSTEP DO SEKCJI 2
        ack2Counter = 0;
        sec2Apply = true;
        for (i = 1; i < 4; i++){
          peopleReqNo = i;
          int data[3];
          data[0] = sectionClock;
          data[1] = APPLY;
          send_to_all(rank, data, sizeof(data), PEOPLE_REQ, size);
          //MPI_Send(&sectionClock, sizeof(int*), MPI_INT, status.MPI_SOURCE, PEOPLE_REQ, MPI_COMM_WORLD);
          sleep(2);
          if (ack2Collected){
            if(CLOCK_DISPLAY) printf("%ld ",clock());
            printf("[CLOCK: %d][ID: %d][SKECJA 2]Uzyskałem dostęp do ludzików.\n", sectionClock, rank);
            peopleAchieved = true;
            peopleReqNo = -1;  
            sectionClock++;//2 punkt osiagniety
            sec2Apply = false;
            //inSec2 = true;
            break;
          }
        }
        if (!peopleAchieved){
          sem_down(semid_sec1, 0);
          for (i = 0; i < (size - 1); i++){
            if (bufferSec1[i].clock > -1){
              int data[3];
              data[0] = bufferSec1[i].clock;
              data[1] = myRoom;
              data[2] = bufferSec1[i].epochNo;
              MPI_Send(data, sizeof(data), MPI_INT, i, ROOM_ACK, MPI_COMM_WORLD);
            }
          }
          sec2Apply = false;
          ack1Collected = false;
          ack2Counter = 0;
          sem_up(semid_sec1, 0);
          

          for (i = 0; i < (size - 1); i++){
            if (bufferSec2[i] > -1){
              int data[3];
              data[0] = bufferSec2[i];
              MPI_Send(data, sizeof(data), MPI_INT, i, PEOPLE_ACK, MPI_COMM_WORLD);
              bufferSec2[i] = -1; //czyszczenie bufora;
            }
          }
          if(CLOCK_DISPLAY) printf("%ld ",clock());
          printf("[CLOCK: %d][ID: %d][SEKCJA 2]Nie udało się uzyskać dostępu do ludzików. Zwalniam sale: %d \n", sectionClock, rank, myRoom);
          myRoom = -1;
          //zwolnic sale!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          sleep(2);
          continue;
        }
        ////////////////////////////////SEKCJA 2
        if ((nWomen > 0) && (nMen > 0)){
          int *assignedPeople = assign_people(nWomen, nMen, rooms[myRoom].size);
          sem_down(semid_sec2, 0);
          myWomen = assignedPeople[0];
          myMen = assignedPeople[1];
          nWomen -= myWomen;
          nMen -= myMen;
          if(CLOCK_DISPLAY) printf("%ld ",clock());
          printf("[CLOCK: %d][ID: %d][SEKCJA 2]Uzyskałem: %d kobiet, %d mężczyzn.\n", sectionClock, rank, myWomen, myMen);
          int data[3];
          data[0] = nWomen;
          data[1] = nMen;
          send_to_all(rank, data, sizeof(data), PEOPLE_NEW_VALUE, size);
          for (i = 0; i < (size-1); i++){
            if (bufferSec2[i] > -1){
              int data[3];
              data[0] = bufferSec2[i];
              MPI_Send(data, sizeof(data), MPI_INT, i, PEOPLE_ACK, MPI_COMM_WORLD);
              bufferSec2[i] = -1; //czyszczenie bufora;
            }
          }
          inSec2 = false;
          ack2Collected = false;
          ack2Counter = 0;
          printf("[CLOCK: %d][ID: %d][SKECJA 2] Zwalniam ludziki.\n", sectionClock, rank);
          sem_up(semid_sec2, 0);
        }
        else{
        	if(CLOCK_DISPLAY) printf("%ld ",clock());
          printf("[CLOCK: %d][ID: %d][SEKCJA 2]BRAK ludzikow\n", sectionClock, rank);
          sem_down(semid_sec1, 0);
          for (i = 0; i < (size-1); i++){
            if (bufferSec1[i].clock > -1){
              int data[3];
              data[0] = bufferSec1[i].clock;
              data[1] = myRoom;
              data[2] = bufferSec1[i].epochNo;
              MPI_Send(data, sizeof(data), MPI_INT, i, ROOM_ACK, MPI_COMM_WORLD);
            }
          }
          sec2Apply = false;
          ack1Collected = false;
          sem_up(semid_sec1, 0);
          

          for (i = 0; i < (size-1); i++){
            if (bufferSec2[i] > -1){
              int data[3];
              data[0] = bufferSec2[i];
              MPI_Send(data, sizeof(data), MPI_INT, i, PEOPLE_ACK, MPI_COMM_WORLD);
              bufferSec2[i] = -1; //czyszczenie bufora;
            }
          }
					if(CLOCK_DISPLAY) printf("%ld ",clock());
          printf("[CLOCK: %d][ID: %d][SEKCJA 2]Nie udało się uzyskać dostępu do ludzików. Zwalniam sale: %d \n", sectionClock, rank, myRoom);
          myRoom = -1;
          //zwolnic sale!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          sleep(2);
          continue;
        }


        //TRWA ORGIA
        if(CLOCK_DISPLAY) printf("%ld ",clock());
        printf("[CLOCK: %d][ID: %d][SEKCJA 2]Trwa orgia...\n", sectionClock, rank);
        sleep(5);
        

        ////////////////////////////////WSTEP DO SEKCJI 2
        sec2Apply = true;
        i = 0;
        while(true){
          i++;
        //for (i = 1; i < 4; i++){
          peopleReqNo = i;
          int data[3];
          data[0] = sectionClock;
          data[1] = RELEASE;
          if(CLOCK_DISPLAY) printf("%ld ",clock());
          printf("[CLOCK: %d][ID: %d][SKECJA 2]WYSYŁAM ŻĄDANIE NA WYJŚCIU.\n", sectionClock, rank);
          send_to_all(rank, data, sizeof(data), PEOPLE_REQ, size);
          //MPI_Send(&sectionClock, sizeof(int*), MPI_INT, status.MPI_SOURCE, PEOPLE_REQ, MPI_COMM_WORLD);
          sleep(2);
          if (ack2Collected){
          	if(CLOCK_DISPLAY) printf("%ld ",clock());
            printf("[CLOCK: %d][ID: %d][SKECJA 2]Uzyskałem dostęp do ludzików.\n", sectionClock, rank);
            peopleAchieved = true;
            peopleReqNo = -1;  
            sectionClock++;//2 punkt osiagniety
            sec2Apply = false;
            ack2Counter = 0;
            //inSec2 = true;
            break;
          }
        }
        
        //WYJSCIE Z SEKCJI 2
        sem_down(semid_sec2, 0);
        int data[3]; //FIFO - nowa wartosc ludzikow przychodzi przed zgoda na wejscie!
        nWomen += myWomen;
        nMen += myMen;
        data[0] = nWomen;
        data[1] = nMen;
        send_to_all(rank, data, sizeof(data), PEOPLE_NEW_VALUE, size);
        for (i = 0; i < (size-1); i++){
          if (bufferSec2[i] > -1){
            int data[3];
            data[0] = bufferSec2[i];
            MPI_Send(data, sizeof(data), MPI_INT, i, PEOPLE_ACK, MPI_COMM_WORLD);
            bufferSec2[i] = -1; //czyszczenie bufora;
          }
        }
        printf("[CLOCK: %d][ID: %d][SKECJA 2] Zwalniam ludziki.\n", sectionClock, rank);
        inSec2 = false;
        ack2Collected = false;
        sem_up(semid_sec2, 0);
        if(CLOCK_DISPLAY) printf("%ld ",clock());
        printf("[CLOCK: %d][ID: %d][SEKCJA 2] Wyszedłem\n", sectionClock, rank);
        ////////////////////////////KONIEC SEKCJI 2
        
        
        //glowna petla (3x) ubiegamy sie o ludziki wysylajac requesty
        //w kazdej petli czekamy na (nr_petli) * size-2 zgod
        //jak uzyskamy dostep to jesli jest >1 K i >1 M to rezerwujemy ludziki
        //jesli nie uzyskamy dostepu, to zwalniamy sale
        //
        //printf("[ID: %d]TUTAJ DRUGA SEKCJA\n", rank);
        ///WYJSCIE Z SEKCJI 1
        //tu  powinien byc semafor
        sem_down(semid_sec1, 0);
        for (i = 0; i < (size-1); i++){
          if (bufferSec1[i].clock > -1){
            int data[3];
            data[0] = bufferSec1[i].clock;
            data[1] = myRoom;
            data[2] = bufferSec1[i].epochNo;
            MPI_Send(data, sizeof(data), MPI_INT, i, ROOM_ACK, MPI_COMM_WORLD);
          }
        }
        ack1Collected = false;
        myRoom = -1;
        sem_up(semid_sec1, 0);
        if(CLOCK_DISPLAY) printf("%ld ",clock());
        printf("[CLOCK: %d][ID: %d][SEKCJA 1]Wyszedłem\n", sectionClock, rank);
        //
      }
      //free(rooms);a
    }
  }
  //printf("[ID: %d]KONIEC\n", rank);
  MPI_Finalize();
  //printf("[ID: %d]KONIEC 2\n", rank);
  exit(0);
}
