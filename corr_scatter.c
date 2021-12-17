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

  if(my_rank!=0){
    int data_count = 1;
    int destination_rank = 0;
    int TAG = 1;
    MPI_Send(&sub_avg, data_count, MPI_FLOAT, destination_rank, TAG, MPI_COMM_WORLD);
  }
  if (my_rank == 0) {    
    float test_avg_float;
    float temp_sum=0;
    MPI_Status status;
    bool passed = false;

    for(int i=1; i< world_size; i++){
        temp_sum = 0;
	for(int j= i*1000; j < ((i+1)*1000) ; j++){
	  temp_sum = temp_sum + rand_nums[j]; 
	}
	float temp_avg = temp_sum/1000;

	MPI_Recv(&test_avg_float, 1, MPI_FLOAT, i,1, MPI_COMM_WORLD, &status);

	if(temp_avg == test_avg_float){
	    passed = true;
	}else{
	    passed = false;
	    //printf
	    break;
        }
    }

    if(passed)
        printf("ARRAY TEST: PASSED\n");
    else
	printf("ARRAY TEST: FAILED\n");

    free(rand_nums);
  }

  free(sub_rand_nums);

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}
