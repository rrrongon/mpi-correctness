#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>

int main(int argc, char* argv[])

{

	int DEBUG_LOG =0; 
	if(argv[1]==NULL)
                DEBUG_LOG = 0;
        else
                DEBUG_LOG = atoi(argv[1]);

	MPI_Init(&argc, &argv);
 
	/*
 * 	declare size of all possible processors
 * 	assign rank to each process
 * 	*/
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
 
	/*
 * 	create reference input array for each process to calculate
 *
 *	0,100,200,300,
 *	400,500,600,700,
 *	800,900,1000,1100,
 *	1200,1300,1400,1500
 *
 *	so for 4 process input would be created such this way
 * */
	int * input_int = malloc(sizeof(int) * size * size);
	int index=0;
	int i;
	for (i=0; i< size; i++){
		int j;
		for (j=0; j< size; j++){
			input_int[index] = i * size * 100 + j*100 ;
	    		index++;
		}
    	}

	/*
 * 	Calculate portion of input for this process
 *	i.e process 1 input is 400,500,600,700
 * */
    	int * my_input = malloc(sizeof(int) * size);
    	for(i = 0; i < size; i++){
        	my_input[i] = my_rank * size * 100 + i * 100;
    	}
	

	if (DEBUG_LOG){
		printf("INFO: RANK %d input: ", my_rank);
		int i;
                for (i=0; i< size; i++){
                        printf(" %d,", my_input[i]);
                }
		printf("\n");
        }

	/*
 *	create receive buffer in each process 
 *	each process will call routine
 *	Each process should receive respective values by alltoall routine in buffer variable.
 * */
        int* buffer_recv = malloc(sizeof(int) * size);
        MPI_Alltoall(my_input, 1, MPI_INT, buffer_recv, 1, MPI_INT, MPI_COMM_WORLD);

    	bool passed = false;

	/*
 *
 *	Check values sent by routine alltoall is as expected for each process
 *	extract expected value from reference array and compare with what received
 * */
    	for (i =0; i < size ; i++){
		int my_buf_val = buffer_recv[i];
		int global_val_pos = i *  size + my_rank ;
		int global_val = input_int[global_val_pos];
		
		if (my_buf_val != global_val){
	    		/*if(DEBUG_LOG)
	        		printf("Value MISMATCHED. Rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, i, my_buf_val, global_val);
			*/
	    		passed = false;
	    		break;
		}else{
	    		/*if(DEBUG_LOG)
	        		printf("MATCHED. Rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, i, my_buf_val, global_val);
			*/
	    		passed = true;
		}

		if(passed){
			if(DEBUG_LOG)
				printf("RANK %d: PASS\n", my_rank);
		}else
			printf(RED "RANK %d: FAIL\n" RESET, my_rank);
    	}

    	if(my_rank==0){
		MPI_Status status;
		bool pass_val_recv = true;
		bool global_decision = true;

		/*
 *
 *		If rank 0 itself passed, ask received value from other processes
 *		Check if processes received expected value.
 * */
		if(passed){
			int rank;
	    		for(rank=1; rank < size; rank++){
	    			MPI_Recv(&pass_val_recv, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &status);
				if(!pass_val_recv){
		    			global_decision = global_decision && pass_val_recv;
					if(DEBUG_LOG)
						printf(RED "TEST: FAIL. RANK %d\n" RESET, rank);
				}else{
					if(DEBUG_LOG)
						printf("TEST: PASS. RANK %d\n", rank);
				}
				
	    		}		

	    		if(global_decision)
				printf("TEST: PASS\n");
	    		else
				printf(RED "TEST: FAIL\n" RESET);
		
		/*Rank 0 already failed*/
		}else{
			/*
 *			Already know test fail. However, need to catch recv routine sent by other processes
 * */
			int rank;
			for(rank=1; rank < size; rank++){
                                MPI_Recv(&pass_val_recv, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &status);
                        }               
	    		printf(RED "TEST: FAIL\n" RESET);
		}

	/*Other processes send their check result*/
    	}else{
		int passed_val = 0;
		if(passed)
	    		passed_val = 1;
		else
	    		passed_val = 0;

		MPI_Send(&passed_val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    	}

	if(size > 4 ){
		/*Divide process by even and odd
 *		create new comm world
 * */
		int color = my_rank % 2;
        	MPI_Comm New_Comm;
        	int new_id, new_world_size, broad_val;

        	MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
        	MPI_Comm_rank(New_Comm, &new_id);
        	MPI_Comm_size(New_Comm, &new_world_size);

		/*sub-comm even,odd*/
		int * sub_input_int = malloc(sizeof(int) * new_world_size * new_world_size);
        	int sub_index=0;
		int i;
        	for (i=0; i< new_world_size; i++){
			int j;
                	for (j=0; j< new_world_size; j++){
                        	sub_input_int[sub_index] = i * new_world_size * 100 + j*100 ;
                        	sub_index++;
                	}
        	}

		int * my_sub_input = malloc(sizeof(int) * new_world_size);
        	for(i = 0; i < new_world_size; i++){
                	my_sub_input[i] = new_id * new_world_size * 100 + i * 100;
        	}

        	if (DEBUG_LOG){
			printf("SUBCOM RANK %d, NEW RANK %d. input: ", my_rank, new_id);
			int i;
                	for (i=0; i< new_world_size; i++){
                        	printf(" %d ", my_sub_input[i]);
                	}
			printf("\n");
        	}

		int* sub_buffer_recv = malloc(sizeof(int) * new_world_size);
        	MPI_Alltoall(my_sub_input, 1, MPI_INT, sub_buffer_recv, 1, MPI_INT, New_Comm);

        	passed = false;

		/*
 *
 *		Each process inside subcom will check it's received value from alltoall routine with
 *		reference input array. Compare what is expected and what process received.
 *
 * 		*/

		for (i =0; i < new_world_size ; i++){
                	int my_sub_buf_val = sub_buffer_recv[i];
                	int global_sub_val_pos = i *  new_world_size + new_id ;
                	int global_sub_val = sub_input_int[global_sub_val_pos];

                	if (my_sub_buf_val != global_sub_val){
                        	if(DEBUG_LOG)
                                	printf(RED "SUBCOMM FAIL:  Rank=%d,newrank=%d, sent: %d, received: %d\n" RESET, my_rank, new_id,  global_sub_val, my_sub_buf_val);
                        	passed = false;
                        	break;
                	}else{
                        	if(DEBUG_LOG)
                                	printf("SUBCOMM Rank=%d, new_rank=%d, sent: %d, received: %d\n", my_rank, new_id, global_sub_val, my_sub_buf_val);
                        	passed = true;
                	}
        	}

		bool sub_global_decision = true;


		/*
 *
 *		Now Each process send it's after check decision to root process to make global decision.
 * 		*/
		if(new_id == 0){
                	MPI_Status status;
                	bool recv_sub_val = true;
                	if(passed){
				int rank;
                        	for(rank=1; rank < new_world_size; rank++){
                                	MPI_Recv(&recv_sub_val, 1, MPI_INT, rank, 0, New_Comm, &status);
                                	if(!recv_sub_val){
                                        	sub_global_decision = sub_global_decision && recv_sub_val;
						if(DEBUG_LOG)
							printf(RED "FAIL: RANK: %d, new rank: %d. Sent:%d, Received:%d\n" RESET, my_rank, new_id );
                                	}
                        	}

                        	/*if(sub_global_decision)
                                	printf("SUBCOMM TEST: PASS\n");
                        	else
                                	printf("SUBCOMM TEST: FAIL");*/
                	}
        	}else{
			/*
 *
 *			Send checked decision to root process inside NEW COMM 
 *
 * 			*/
                	int sub_passed_val = 0;
                	if(passed)
                        	sub_passed_val = 1;
                	else
                        	sub_passed_val = 0;

                	MPI_Send(&sub_passed_val, 1, MPI_INT, 0, 0, New_Comm);
        	}

		if(my_rank == 0 && new_id == 0){
			/*Receives decision from root which is not root for WORLDCOMM*/
			MPI_Status status;
			int recv_subcom_val;
			MPI_Recv(&recv_subcom_val, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

			if(sub_global_decision && recv_subcom_val)
				printf("SUBCOM TEST: PASS\n");
			else
				printf(RED"SUBCOM TEST: FAIL\n"RESET);
	
		}else if(new_id == 0 && my_rank != 0){
			/*
 *
 *			one root of NEWCOMM but not root in WORLD COMM sends decision to WORLDCOMM root
 *
 * */
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

