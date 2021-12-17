#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>

float *create_rand_nums(int num_elements) {
  float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
  int i;
  for (i = 0; i < num_elements; i++) {
    rand_nums[i] = (rand() / (float)RAND_MAX);
  }
  return rand_nums;
}


int main(int argc, char** argv) {

  int num_elements_per_proc = 1000; //atoi(argv[1]);

  MPI_Init(NULL, NULL);

  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);


  srand(time(NULL)*my_rank); // Seed the random number generator of processes uniquely
  float *rand_nums = NULL;
  rand_nums = create_rand_nums(num_elements_per_proc);

  float local_sum = 0;
  int i;
  for (i = 0; i < num_elements_per_proc; i++) {
    local_sum += rand_nums[i];
  }

  float global_sum;
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM,
                MPI_COMM_WORLD);

  if (my_rank==0){
    float global_sum_recv = 0;
    MPI_Status status;
   
    bool passed = false;
 
    for (int rank=1; rank < world_size; rank++){
      MPI_Recv(&global_sum_recv, 1, MPI_FLOAT, rank, 1, MPI_COMM_WORLD, &status);
      if (global_sum_recv == global_sum){
	passed = true;
      }else{
	passed = false;
	break;
      }
    }

    if(passed)
	printf("PASSED\n");
    else
	printf("FAILED\n");
  }else{
    MPI_Send(&global_sum, 1, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

}
