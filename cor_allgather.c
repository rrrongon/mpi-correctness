#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

float *create_rand_nums(int num_elements) {
  float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
  int i;
  for (i = 0; i < num_elements; i++) {
    rand_nums[i] = (rand() / (float)RAND_MAX);
  }
  return rand_nums;
}


float compute_avg(float *array, int num_elements) {
  float sum = 0.f;
  int i;
  for (i = 0; i < num_elements; i++) {
    sum += array[i];
  }
  return sum / num_elements;
}


int main(int argc, char** argv) {
  
  int DEBUG_LOG =0; 
  DEBUG_LOG = atoi(argv[1]);

  int num_elements_per_proc = 1000;//atoi(argv[1]);

  srand(time(NULL));

  MPI_Init(NULL, NULL);

  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  float *rand_nums = NULL;
  if (my_rank == 0) {
    rand_nums = create_rand_nums(num_elements_per_proc * world_size);
  }

  float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);


  MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums,
              num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

  float sub_avg = compute_avg(sub_rand_nums, num_elements_per_proc);

  float *sub_avgs = (float *)malloc(sizeof(float) * world_size);
  MPI_Allgather(&sub_avg, 1, MPI_FLOAT, sub_avgs, 1, MPI_FLOAT, MPI_COMM_WORLD);

  float avg = compute_avg(sub_avgs, world_size);

  if(DEBUG_LOG)
    printf("Avg of all elements from proc %d is %f\n", my_rank, avg);

  MPI_Barrier(MPI_COMM_WORLD);

  if (my_rank == 0) {

    float all_value_avg = compute_avg(rand_nums, num_elements_per_proc * world_size);
    
    MPI_Status status;
    int global_pass = 1;
    float avg_recv = 0;
    int local_pass = 1;
    float diff = avg - all_value_avg;

    if (diff >0.001){
      global_pass = 0;
      if(DEBUG_LOG)
        printf("avg did not match. allgather avg=%f, calculated avg=%f\n", avg, all_value_avg );
    }else{
      if(DEBUG_LOG)
        printf("avg is EQUAL. allgather avg=%f, calculated avg=%f\n", avg, all_value_avg);
    }

    for(int rank=1; rank < world_size; rank++){

	MPI_Recv(&avg_recv, 1, MPI_FLOAT, rank, 0, MPI_COMM_WORLD, &status);
	diff = all_value_avg - avg_recv;
	if (diff < 0)
	    diff = diff * (-1);
	if(diff > 0.001)
	    local_pass = 0;
	else
	    local_pass = 1;

	global_pass = global_pass && local_pass;

    }

    free(rand_nums);

    if(global_pass)
	printf("TEST: PASS\n");
    else
	printf("TEST: FAIL\n");

  }
  else
  {
    MPI_Send(&avg, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
  }

  free(sub_avgs);
  free(sub_rand_nums);

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return EXIT_SUCCESS;
}
