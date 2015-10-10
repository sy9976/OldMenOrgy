#include "semaphores.h"


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
