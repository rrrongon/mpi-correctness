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
    for (int i = 0; i < num_elements_per_proc; i++) {
        local_sum += rand_nums[i];
    }

    float global_sum;
    int root = 0;
	
	int global_pass = 0;

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	if(my_rank ==0 ){
		MPI_Status status;
		float cal_global_sum=0;
		for(int rank=1; rank< world_size; rank++){
			float *recv_nums = (float *)malloc(sizeof(float) * 1000);;
			float cal_local_sum = 0;
        		MPI_Recv (recv_nums, 1000, MPI_FLOAT, rank, 1, MPI_COMM_WORLD, &status);
			for (int j=0; j< 1000; j++){
				cal_local_sum = cal_local_sum + recv_nums[j];
			}
			cal_global_sum = cal_global_sum +  cal_local_sum;
		}

		cal_global_sum = cal_global_sum + local_sum;
			
		float diff = global_sum - cal_global_sum;
		if(diff <0)
			diff = diff * (-1);

		if(diff>0.001)
			global_pass = 0;
		else
			global_pass = 1;

		if(global_pass)
			printf("TEST: PASS\n");	

	}else{
		for(int x=0;x< 10; x++)
		MPI_Send(rand_nums, num_elements_per_proc , MPI_FLOAT, 0, 1, MPI_COMM_WORLD); 
	}

    free(rand_nums);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}
