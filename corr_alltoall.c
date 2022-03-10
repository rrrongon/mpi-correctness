#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>

#define SIZES 10

int main(int argc, char* argv[])

{

	int DEBUG_LOG =0; 
	if(argv[1]==NULL)
                DEBUG_LOG = 0;
        else
                DEBUG_LOG = atoi(argv[1]);

	int sizes[SIZES] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
    int size_counter;

	MPI_Init(&argc, &argv);
 
	/*
 * 	declare size of all possible processors
 * 	assign rank to each process
 * 	*/
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
 
	for(size_counter=0; size_counter<SIZES; size_counter++){
		
		int i;
		int num_elements_per_proc = sizes[size_counter] * 1024;
		
		int data_cnt_global = num_elements_per_proc * world_size * world_size;
		float * global_ref_data = malloc(sizeof(float) * data_cnt_global);
		//printf("global data in rank: %d \n", my_rank);
		for(i = 0; i < data_cnt_global; i++){
			global_ref_data[i] = i;
			//if(my_rank==0 )
			//	printf(" %f,",global_ref_data[i]);
		}
		//if(my_rank==0)
		//printf("\n");

		int data_cnt_local = num_elements_per_proc * world_size;
		float * local_data = malloc(sizeof(float) * data_cnt_local);
		int start_pos = my_rank * num_elements_per_proc *  world_size;
		int index=0;
		//printf("local data in rank: %d \n", my_rank);
		for(i = start_pos; i < start_pos + data_cnt_local; i++){
			local_data[index] = i;
			index++;
			//printf(" rank %d : %d\n", my_rank, i);
		}
                //printf("\n");

		// call routine
		float * recv_buf = malloc(sizeof(float) * data_cnt_local);
		MPI_Alltoall(local_data, num_elements_per_proc, MPI_FLOAT, recv_buf, num_elements_per_proc, MPI_FLOAT, MPI_COMM_WORLD);

		/*Create expected data array for this rank*/
		float * expected_buf = malloc(sizeof(float) * data_cnt_local);
		int local_st_pos, j;
		index = 0;
		//printf("expected data in rank: %d \n", my_rank);
		for(i=0;i< world_size; i++){
			local_st_pos = (i * num_elements_per_proc * world_size) + (my_rank * num_elements_per_proc);
			for(j= local_st_pos; j< local_st_pos+num_elements_per_proc; j++){
				expected_buf[index] = global_ref_data[j];
				/*if(my_rank==0){
					printf("pos: %d, gl_val: %f, ex_val: %f",j, global_ref_data[j], expected_buf[index]);
				}*/
				index++;
			}
			//if(my_rank==0)
				//printf("val of i: %d\n",i);
		}

		int local_pass=1;
		for(i=0;i<data_cnt_local; i++){
			float recv_val = recv_buf[i];
			float exp_val = expected_buf[i];
			//if(my_rank==0)
			//	printf("recv: %f, exp: %f\n", recv_val,exp_val);
			if(recv_val != exp_val){
				local_pass = local_pass && 0;
			}else{
				local_pass = local_pass && 1;
			}
		}
		free(recv_buf);
		free(expected_buf);
		free(global_ref_data);

		if(my_rank!=0){
			MPI_Send(&local_pass, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		}else{
			int rank, recv_local_pass, global_pass=1;
			MPI_Status status;
			for(rank=1; rank<world_size;rank++){
				MPI_Recv(&recv_local_pass, 1, MPI_INT, rank, 1, MPI_COMM_WORLD, &status);
				global_pass = global_pass && recv_local_pass;
				if(!recv_local_pass){
					printf(RED"TEST: FAIL. rank %d\n"RESET, rank);
					global_pass = global_pass && recv_local_pass;
				}else{
					if(DEBUG_LOG){
						printf("TEST: PASS. rank %d\n",rank);
					}
				}
			}
			if(global_pass)
				printf("MSG SIZE: %d*1024, PASS\n",sizes[size_counter]);
			else
				printf("MSG SIZE: %d*1024, FAIL\n",sizes[size_counter]);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
  	MPI_Finalize();
	return EXIT_SUCCESS;
}

