#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 
#include <time.h>
#include <stdio.h>
#include  <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SIZES 10
#define RAND 1000

/*Generating random numbers for input*/
float *create_rand_nums(int num_elements) {
        float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
        int i;
	float temp;
        for (i = 0; i < num_elements; i++) {
		temp = (rand() % RAND);
		temp = round(temp);
                rand_nums[i] = temp;
        }
        return rand_nums;
}


int main(int argc, char** argv){

	int DEBUG_LOG;
	if(argv[1]==NULL)
		DEBUG_LOG = 0;
	else
		DEBUG_LOG = atoi(argv[1]);

	MPI_Init(NULL, NULL);

	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if(world_size>2){
		printf(RED"MUST BE ONLY 2 PROCESSES. USE n -2\n"RESET);
		return 0;
	}

	int sizes[SIZES] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
	int size_counter;

	for(size_counter=0; size_counter<SIZES; size_counter++){
		int num_elements_per_proc = sizes[size_counter] * 1024;
		
		float *rand_nums = NULL;
		if (my_rank == 0) {
				int dest_rank=1;
				int from_rank = 1;
				int tag = 0;
				float *recv_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);

				rand_nums = create_rand_nums(num_elements_per_proc);
				MPI_Send(rand_nums, num_elements_per_proc, MPI_FLOAT, dest_rank, tag, MPI_COMM_WORLD);
				MPI_Recv (recv_nums, num_elements_per_proc, MPI_FLOAT, from_rank, tag, MPI_COMM_WORLD, &status);

				int j;
				int rank_pass = 1;
				for(j=0;j<num_elements_per_proc;j++){
					float gen_val = rand_nums[j];
					float recv_val =  recv_nums[j];

					if(gen_val!=recv_val){
						printf(RED"EXPECTED %f, RECEIVED %f at position %d\n"RESET, gen_val,recv_val, j);
						rank_pass = rank_pass && 0;
					}else{
						rank_pass = rank_pass && 1;
					}
				}

				if(rank_pass)
					printf("TEST: PASS for SIZE:%d*1024\n",size_counter);
				else
					printf(RED"TEST: FAIL for SIZE:%d*1024"RESET, size_counter);
		}else{
			MPI_Status status;
			int from_rank = 0;
			int dest_rank = 0;
			int tag = 0;
			rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);
        	MPI_Recv ( rand_nums, num_elements_per_proc, MPI_FLOAT, from_rank, tag, MPI_COMM_WORLD, &status);
			MPI_Send(rand_nums, num_elements_per_proc, MPI_FLOAT, dest_rank, tag, MPI_COMM_WORLD);
		}
	}

    MPI_Finalize();

    return 0;
}
