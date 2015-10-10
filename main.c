#include "parameters.h"


int main(int argc, char **argv) {
  if (argc < 4) {
    printf("not enough parameters\n");
    return 0;
  }
  printf("generating rooms sizes\n");
  bool correct = check_parameters_correctness(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
  printf("Parameters correct: %d\n",correct);
  if (correct)
  {
    Room* rooms = generate_rooms(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    int i;
    for (i=0; i<=atoi(argv[1])-1; i++)
    {
      printf("Room[%d].size = %d\n", i, rooms[i].size);
    }
  }
  return 0;
}
