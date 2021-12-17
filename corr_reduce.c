#include <time.h>
#include <stdio.h>
#include  <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
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

    int num_elements_per_proc = 1000;

    MPI_Init(NULL, NULL);

    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    srand(time(NULL)*my_rank);
    float *rand_nums = NULL;
    rand_nums = create_rand_nums(num_elements_per_proc);

    float local_sum = 0;
    int i;
    for (i = 0; i < num_elements_per_proc; i++) {
        local_sum += rand_nums[i];
    }

    float global_sum;
    int root = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, root, MPI_COMM_WORLD);

    printf("reduced\n");

    MPI_Barrier(MPI_COMM_WORLD);

    if(my_rank!=0 ){
	int data_count = num_elements_per_proc;
        int destination_rank = 0;
        int TAG = 1;
        MPI_Send(&rand_nums, data_count, MPI_FLOAT, destination_rank, 0, MPI_COMM_WORLD);
	printf("SENT from rank %d\n", my_rank);
    }else if (my_rank==0){
        printf("reduced global sum %f\n", global_sum);
	MPI_Status status;
	float total_sum = 0;
	float *rand_nums_recv = (float *)malloc(sizeof(float) * num_elements_per_proc);

	for (int x = 1; x < world_size; x++){
	    float *rand_nums_recv = (float *)malloc(sizeof(float) * num_elements_per_proc);
	    MPI_Recv(&rand_nums_recv, num_elements_per_proc, MPI_FLOAT, x,0, MPI_COMM_WORLD, &status);
	    printf("received from %d\n", x);
	}

	total_sum = total_sum + local_sum;

	if(total_sum == global_sum){
	    printf("TEST: PASSED");
	}else{
	    printf("TEST: FAILED");
	}

	free(rand_nums_recv);
    }

    free(rand_nums);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}
