#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

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

int main(int argc, char* argv[]){
    
	int DEBUG_LOG =0; 
	if(argv[1]==NULL)
		DEBUG_LOG = 0;
	else
		DEBUG_LOG = atoi(argv[1]);

	int sizes[SIZES] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
	int size_counter;

	MPI_Init(NULL,NULL);

    	int size;
    	MPI_Comm_size(MPI_COMM_WORLD, &size);
    	int my_rank;
    	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	for (size_counter=0; size_counter<SIZES; size_counter++){
		int root=0;
		/*Broadcast some large int value*/
		int min_int;
		int num_elements_per_proc = sizes[size_counter] * 1024;
		float *rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);
		if(my_rank==0){
			free(rand_nums);
			rand_nums = create_rand_nums(num_elements_per_proc);
		}
				
		int data_count = num_elements_per_proc;
		MPI_Bcast(rand_nums, data_count, MPI_FLOAT, root, MPI_COMM_WORLD);	
				
		int global_pass = 1;
			
		if(my_rank==0){
			MPI_Status status;
			float *min_val_recv = (float *)malloc(sizeof(float) * num_elements_per_proc);
			
			bool rank_pass;
			/*Ask from all process what they received via broadcast routine
			Check if received value is equal to what was sent by root
			*/
			int rank;
			for(rank=1;rank< size; rank++){
				rank_pass = true;
	    			MPI_Recv(min_val_recv, data_count, MPI_FLOAT, rank,1, MPI_COMM_WORLD, &status);
	    			int j;
				for(j=0;j<data_count;j++){
					float expected_val = rand_nums[j];
					float recv_val = min_val_recv[j];

					if(expected_val!=recv_val){
						rank_pass = rank_pass && 0;
						printf(RED"RANK %d, at position %d: EXPECTED %f , ROUTINE %f \n"RESET, rank,j, expected_val,recv_val);
					}else{
						rank_pass = rank_pass && 1;
					}
				}
				if(rank_pass){
					if(DEBUG_LOG)
						printf("TEST: PASS. RANK %d for size: %d * 1024\n",rank, sizes[size_counter]);
				}else{
					printf(RED"TEST: FAIL. RANK for size: %d * 1024\n"RESET,rank, sizes[size_counter]);
				}

				global_pass = global_pass && rank_pass;
			if(global_pass)
				printf("TEST: PASS for size: %d * 1024\n",sizes[size_counter]);
			else
				printf(RED"TEST: FAIL for size: %d\n"RESET, sizes[size_counter]);
			}
		}else{
			/*Send received value via routine to root for cross-match*/
			MPI_Status status;
		
			int destination_rank=0;
			MPI_Send(rand_nums, data_count, MPI_FLOAT, destination_rank, 1, MPI_COMM_WORLD);	
		}

		/*if(size > 4){
			//Create New world
			int color = my_rank % 2;
			MPI_Comm New_Comm;
			int new_id, new_world_size, broad_val;

			MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
			MPI_Comm_rank(New_Comm, &new_id);
			MPI_Comm_size(New_Comm, &new_world_size);

			float *subcomm_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);
			if(new_id==0){
				free(subcomm_rand_nums);
				subcomm_rand_nums = create_rand_nums(num_elements_per_proc);
			}

			int data_count = num_elements_per_proc;
			int new_root = 0;
			MPI_Bcast(subcomm_rand_nums, data_count, MPI_FLOAT, new_root, New_Comm);
	
			int subcomm_global_pass=1;
				
			if(new_id==0){
				MPI_Status status;
				float *subcomm_recv_val = (float *)malloc(sizeof(float) * num_elements_per_proc);
	
				int rank;
				bool rank_pass;
				for(rank=1;rank< new_world_size; rank++){
					rank_pass = true;
					MPI_Recv(subcomm_recv_val, data_count, MPI_FLOAT, rank,1, New_Comm, &status);
					if(DEBUG_LOG)
						printf("recv data from subcom new rank %d\n", new_id);	
					int j;
					for(j=0;j<num_elements_per_proc; j++){		
						float expected_val = subcomm_rand_nums[j];
						float recv_val = subcomm_recv_val[j];

						if(expected_val!=recv_val){
							rank_pass = rank_pass && 0;
							printf(RED"SUBCOMM NEWRANK %d, at position %d: EXPECTED %f , ROUTINE %f \n"RESET, rank,j, expected_val,recv_val);
						}else{
							rank_pass = rank_pass && 1;
						}
					}
					if(rank_pass){
						if(DEBUG_LOG)
							printf("SUMCOMM TEST: PASS. NEWRANK %d\n",rank);
					}else{
						printf(RED"SUBCOMM TEST: FAIL. NEWRANK %d\n"RESET, rank);
					}

					subcomm_global_pass = subcomm_global_pass && rank_pass;
				}

				if(my_rank == 0){
                                        
                                	MPI_Status status;
					int recv_subcom_pass;
					MPI_Recv(&recv_subcom_pass, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

                                        if(recv_subcom_pass && subcomm_global_pass )
                                                printf("SUBCOMM TEST: PASS\n");
                                        else
                                                printf(RED"SUBCOMM TEST: FAIL \n"RESET);
				}else{
					MPI_Send(&subcomm_global_pass, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				}

				free(subcomm_recv_val);
			}else{
				int destination_rank=0;
				MPI_Send(subcomm_rand_nums, num_elements_per_proc, MPI_FLOAT, destination_rank, 1,New_Comm);	
			}
					
		}*/

		free(rand_nums);
	}

    	MPI_Finalize();
        
    	return EXIT_SUCCESS;
 }
