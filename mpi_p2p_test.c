#include <time.h>
#include <stdio.h>
#include  <mpi.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char** argv){
    MPI_Init(NULL, NULL);

    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int passing_value = -1;
    int received_val;

    double balance[5] = {1000.0, 2.0, 3.4, 7.0, 50.0};
    double rec_array[5];

    if(my_rank==0){
        int destination_rank;
        int count=1;

        for (int i=1;i<world_size; i++){
	    destination_rank=i;
	    MPI_Send(&passing_value, count, MPI_INT, destination_rank, 0, MPI_COMM_WORLD);
        }
        
	for (int i=1; i< world_size; i++){
	    destination_rank=i;
            MPI_Send(&balance, 5, MPI_DOUBLE, destination_rank, 1, MPI_COMM_WORLD); 
	}
       
        
    }else if (my_rank!=0){

        MPI_Status status;

        MPI_Recv ( &received_val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG==0){
            if (received_val != passing_value)
                printf("Value did not match. Sent value %d, received value %d in rank %d \n", passing_value, received_val, my_rank) ;
            else
                printf("Value matched in rank %d. Sent %d, Received %d \n",my_rank,  passing_value, received_val);
        }

        MPI_Recv(&rec_array, 5, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG==1){
	    for (int j=0; j<5; j++){
		if(balance[j]==rec_array[j])
		    printf("Matched double vals: %lf on rank %d\n", rec_array[j], my_rank);
		else
		    printf("---Mismatched val: on position %d, on rank %d, sent %lf, received %lf", j, my_rank, balance[j], rec_array[j]);
	    }
	}

    }

    MPI_Finalize();

    return 0;
}
