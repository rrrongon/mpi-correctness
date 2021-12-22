#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>

int main(int argc, char* argv[])

{

	int DEBUG_LOG =0; 
	DEBUG_LOG = atoi(argv[1]);

	MPI_Init(&argc, &argv);
 
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
 
	int * input_int = malloc(sizeof(int) * size * size);
	int index=0;
	for (int i=0; i< size; i++){
		for (int j=0; j< size; j++){
			input_int[index] = i * size * 100 + j*100 ;
	    		index++;
		}
    	}

    	int * my_input = malloc(sizeof(int) * size);
    	for(int i = 0; i < size; i++){
        	my_input[i] = my_rank * size * 100 + i * 100;
    	}
	

	if (DEBUG_LOG){
                for (int i=0; i< size; i++){
                        printf("Process %d, my values = %d\n", my_rank, my_input[i]);
                }
        }

        int* buffer_recv = malloc(sizeof(int) * size);
        MPI_Alltoall(my_input, 1, MPI_INT, buffer_recv, 1, MPI_INT, MPI_COMM_WORLD);

    	bool passed = false;

    	for (int i =0; i < size ; i++){
		int my_buf_val = buffer_recv[i];
		int global_val_pos = i *  size + my_rank ;
		int global_val = input_int[global_val_pos];

		if (my_buf_val != global_val){
	    		if(DEBUG_LOG)
	        		printf("Value MISMATCHED. Rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, i, my_buf_val, global_val);
	    		passed = false;
	    		break;
		}else{
	    		if(DEBUG_LOG)
	        		printf("Value MATCHED. Rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, i, my_buf_val, global_val);
	    		passed = true;
		}

	//printf("Values collected on process %d: %d\n", my_rank, buffer_recv[i]);
    	}

    	if(my_rank==0){
		MPI_Status status;
		bool pass_val_recv = true;
		bool global_decision = true;
		if(passed){
	    		for(int rank=1; rank < size; rank++){
	    			MPI_Recv(&pass_val_recv, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &status);
				if(!pass_val_recv){
		    			global_decision = global_decision && pass_val_recv;
				}
	    		}		

	    		if(global_decision)
				printf("GLOBAL TEST: PASS\n");
	    		else
				printf("GLOBAL TEST: FAIL");
		}else{
	    		printf("TEST: FAIL\n");
		}
    	}else{
		int passed_val = 0;
		if(passed)
	    		passed_val = 1;
		else
	    		passed_val = 0;

		MPI_Send(&passed_val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    	}

	if(size > 4 ){	
		int color = my_rank % 2;
        	MPI_Comm New_Comm;
        	int new_id, new_world_size, broad_val;

        	MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
        	MPI_Comm_rank(New_Comm, &new_id);
        	MPI_Comm_size(New_Comm, &new_world_size);

		/*sub-comm even,odd*/
		int * sub_input_int = malloc(sizeof(int) * new_world_size * new_world_size);
        	int sub_index=0;
        	for (int i=0; i< new_world_size; i++){
                	for (int j=0; j< new_world_size; j++){
                        	sub_input_int[sub_index] = i * new_world_size * 100 + j*100 ;
                        	sub_index++;
                	}
        	}

		int * my_sub_input = malloc(sizeof(int) * new_world_size);
        	for(int i = 0; i < new_world_size; i++){
                	my_sub_input[i] = new_id * new_world_size * 100 + i * 100;
        	}

        	if (DEBUG_LOG){
                	for (int i=0; i< new_world_size; i++){
                        	printf("Process %d, my values = %d\n", new_id, my_sub_input[i]);
                	}
        	}

		int* sub_buffer_recv = malloc(sizeof(int) * new_world_size);
        	MPI_Alltoall(my_sub_input, 1, MPI_INT, sub_buffer_recv, 1, MPI_INT, New_Comm);

        	passed = false;

		for (int i =0; i < new_world_size ; i++){
                	int my_sub_buf_val = sub_buffer_recv[i];
                	int global_sub_val_pos = i *  new_world_size + new_id ;
                	int global_sub_val = sub_input_int[global_sub_val_pos];

                	if (my_sub_buf_val != global_sub_val){
                        	if(DEBUG_LOG)
                                	printf("SUBCOMM Value MISMATCHED. Rank=%d,new_rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, new_id, i, my_sub_buf_val, global_sub_val);
                        	passed = false;
                        	break;
                	}else{
                        	if(DEBUG_LOG)
                                	printf("SUBCOMM Value MATCHED. Rank=%d, new_rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, new_id, i, my_sub_buf_val, global_sub_val);
                        	passed = true;
                	}
        	}

		bool sub_global_decision = true;

		if(new_id == 0){
                	MPI_Status status;
                	bool recv_sub_val = true;
                	if(passed){
                        	for(int rank=1; rank < new_world_size; rank++){
                                	MPI_Recv(&recv_sub_val, 1, MPI_INT, rank, 0, New_Comm, &status);
                                	if(!recv_sub_val){
                                        	sub_global_decision = sub_global_decision && recv_sub_val;
                                	}
                        	}

                        	/*if(sub_global_decision)
                                	printf("SUBCOMM TEST: PASS\n");
                        	else
                                	printf("SUBCOMM TEST: FAIL");*/
                	}
        	}else{
                	int sub_passed_val = 0;
                	if(passed)
                        	sub_passed_val = 1;
                	else
                        	sub_passed_val = 0;

                	MPI_Send(&sub_passed_val, 1, MPI_INT, 0, 0, New_Comm);
        	}

		if(my_rank == 0 && new_id == 0){
			MPI_Status status;
			int recv_subcom_val;
			MPI_Recv(&recv_subcom_val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

			if(sub_global_decision && recv_subcom_val)
				printf("SUBCOM TEST: PASS\n");
			else
				printf("SUBCOM TEST: FAIL\n");
	
		}else if(new_id == 0 && my_rank != 0){
			if (sub_global_decision){
				int pass = 1;
				MPI_Send(&pass, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			}else{
				int pass = 0;
				MPI_Send(&pass, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			}
		}

 	}

   	MPI_Finalize();
 
    	return EXIT_SUCCESS;
}

