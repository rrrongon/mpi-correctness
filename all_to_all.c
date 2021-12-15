#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char* argv[])

{
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
    for (int i=0; i< size; i++)
    printf("Process %d, my values = %d\n", my_rank, my_values[i]);
 
    int* buffer_recv = malloc(sizeof(int) * size);
    MPI_Alltoall(my_values, 1, MPI_INT, buffer_recv, 1, MPI_INT, MPI_COMM_WORLD);

    for (int i =0; i < size ; i++){
	int my_buf_val = buffer_recv[i];
	int global_val_pos = i *  size + my_rank ;
	int global_val = all_values[global_val_pos];

	if (my_buf_val != global_val){
	    printf("Value MISMATCHED. Rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, i, my_buf_val, global_val);
	}else{
	    printf("Value MATCHED. Rank=%d, position=%d, received val=%d, actual val = %d\n", my_rank, i, my_buf_val, global_val);
	}

	//printf("Values collected on process %d: %d\n", my_rank, buffer_recv[i]);
    }
 
    MPI_Finalize();
 
    return EXIT_SUCCESS;
}

