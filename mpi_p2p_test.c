#include <time.h>
#include <stdio.h>
#include  <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

    char test_string[1000] ="Creates new communicators based on colors and keys. Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys";
    char rec_string[1000];

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
       
        for (int i=1; i< world_size; i++){
            destination_rank=i;
            MPI_Send(&test_string, strlen(test_string), MPI_CHAR, destination_rank, 2, MPI_COMM_WORLD); 
        }
 
    }else if (my_rank!=0){

        MPI_Status status;

        MPI_Recv ( &received_val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG==0){
            if (received_val != passing_value)
                printf("Value did not match. Sent value %d, received value %d in rank %d \n", passing_value, received_val, my_rank) ;
            else{
                    //printf("Value matched in rank %d. Sent %d, Received %d \n",my_rank,  passing_value, received_val);
		    printf("Test Passed: INT, RANK=%d\n", my_rank);
		}
        }

        MPI_Recv(&rec_array, 5, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);

	bool mismatched = false;
        if(status.MPI_TAG==1){
	    for (int j=0; j<5; j++){
		if(balance[j]==rec_array[j]){
		    //printf("Matched double vals: %lf on rank %d\n", rec_array[j], my_rank);    
		}
		else{
		    printf("---Mismatched val: on position %d, on rank %d, sent %lf, received %lf", j, my_rank, balance[j], rec_array[j]);
		    mismatched = true;
		    break;
	    }
	}
	    if(mismatched){
		printf("Test failed: Double Array, Rank=%d\n", my_rank);
	    }else{
		printf("Test Passed: DOUBLE Array, Rank=%d\n", my_rank);
	    }

    }

    MPI_Recv(&rec_string, strlen(test_string), MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);

        mismatched = false; 
        if(status.MPI_TAG==2){
            for (int j=0; j<strlen(test_string); j++){
                if(test_string[j]==rec_string[j]){
                    //printf("Matched char vals: %c on rank %d\n", rec_string[j], my_rank);     
                }
                else{
                    printf("---Mismatched val: on position %d, on rank %d, sent %c, received %c", j, my_rank, test_string[j], rec_string[j]);
                    mismatched = true;
                    break;
            }
        }
            if(mismatched){
                printf("Test failed: Double Array, Rank=%d\n", my_rank);
            }else{
		printf("Test Passed: Char array, Rank=%d\n", my_rank);
	    }
 
    }

    }

    MPI_Finalize();

    return 0;
}
