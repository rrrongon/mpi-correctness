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
 
    int * all_values = malloc(sizeof(int) * size * size);
    int index=0;
    for (int i=0; i< size; i++){
	for (int j=0; j< size; j++){
	    all_values[index] = i * size * 100 + j*100 ;
	    index++;
	}
    }

    int * my_values = malloc(sizeof(int) * size);
    for(int i = 0; i < size; i++)
    {
        my_values[i] = my_rank * size * 100 + i * 100;
    }

    if (DEBUG_LOG){
        for (int i=0; i< size; i++){
             printf("Process %d, my values = %d\n", my_rank, my_values[i]);
        }
    }

    int* buffer_recv = malloc(sizeof(int) * size);
    MPI_Alltoall(my_values, 1, MPI_INT, buffer_recv, 1, MPI_INT, MPI_COMM_WORLD);

    bool passed = false;

    for (int i =0; i < size ; i++){
	int my_buf_val = buffer_recv[i];
	int global_val_pos = i *  size + my_rank ;
	int global_val = all_values[global_val_pos];

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
		printf("TEST: PASS\n");
	    else
		printf("TEST: FAIL");
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
 
    MPI_Finalize();
 
    return EXIT_SUCCESS;
}

