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

	int test_int = -1;
	int rec_val;

	double test_array[5] = {1000.0, 2.0, 3.4, 7.0, 50.0};
	double rec_array[5];

	char test_string[1000] ="Creates new communicators based on colors and keys. Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys";
	char rec_string[1000];

	if(my_rank==0){
		int dest_rank;
		int data_cnt = 1;

		MPI_Status status;
		int tag;
		int global_dec = 1;
        	for (int i=1;i< world_size; i++){
			tag = 3;
	    		dest_rank = i;
			int local_dec = 1;
	    		MPI_Send(&test_int, data_cnt, MPI_INT, dest_rank, 0, MPI_COMM_WORLD);
			MPI_Recv ( &rec_val, data_cnt, MPI_INT, i, tag, MPI_COMM_WORLD, &status);

			if(rec_val!=test_int){
				printf( RED "TEST INT: FAIL. Sent value %d, Received value %d in rank %d \n" RESET , test_int, rec_val, i) ;
				local_dec = 0;
			}else{
				local_dec = 1;
			}

			global_dec = global_dec && local_dec;
        	}
        
		if(!global_dec)
			printf("INT TEST: FAIL\n");
		
		double rec_array[5];
		int arr_dec = 1;
		for (int i=1; i< world_size; i++){
	    		dest_rank=i;
            		MPI_Send(&test_array, 5, MPI_DOUBLE, dest_rank, 1, MPI_COMM_WORLD); 

			data_cnt = 5;
			tag = 4;
			MPI_Recv ( &rec_array, data_cnt, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status);

			int local_dec = 1;
			for(int j=0; j< 5; j++){
				if(test_array[j]!= rec_array[j]){
					local_dec = 0; 
				}
			}
			
			if(!local_dec)
				printf(RED "TEST ARRAY: FAIL. Rank %d\n"RESET, i);

			arr_dec = arr_dec && local_dec;

		}
		
		global_dec = global_dec && arr_dec;
		if(!arr_dec)
			printf(RED "ARRAY TEST: FAIL\n"RESET);
       
        	for (int i=1; i< world_size; i++){
            		dest_rank=i;
            		MPI_Send(&test_string, 1000, MPI_CHAR, dest_rank, 2, MPI_COMM_WORLD); 

			tag = 5;
			MPI_Recv ( &rec_string, 1000, MPI_CHAR, i, tag, MPI_COMM_WORLD, &status);

                        int local_dec = 1;
                        for(int j=0; j< 5; j++){
                                if(test_string[j]!= rec_string[j]){
                                        local_dec = 0;
                                }
                        }

                        if(!local_dec)
                                printf(RED "TEST STRING: FAIL. Rank %d\n"RESET, i);

                        global_dec = global_dec && local_dec;

        	}

		if(global_dec)
			printf("TEST: PASS\n");
		else
			printf(RED "TEST: FAIL\n" RESET);
 
    	}else if (my_rank!=0){

        	MPI_Status status;
		int data_cnt = 1;
		int from_rank = 0;
		int tag = 0;

        	MPI_Recv ( &rec_val, data_cnt, MPI_INT, from_rank, tag, MPI_COMM_WORLD, &status);
		
		int dest_rank = 0;
		tag = 3;
		MPI_Send(&rec_val, data_cnt, MPI_INT, dest_rank, tag, MPI_COMM_WORLD);

        	data_cnt = 5;
		tag = 1;
		MPI_Recv(&rec_array, data_cnt, MPI_DOUBLE, from_rank, tag, MPI_COMM_WORLD, &status);
		
		tag = 4;
		MPI_Send(&rec_array, data_cnt, MPI_DOUBLE, dest_rank, tag, MPI_COMM_WORLD);
        
    		tag = 2;
		MPI_Recv(&rec_string, 1000, MPI_CHAR, from_rank, tag, MPI_COMM_WORLD, &status);
		
		tag = 5;
		data_cnt = 1000;
		MPI_Send(&rec_string, 1000, MPI_CHAR, dest_rank, tag, MPI_COMM_WORLD);

    }

    MPI_Finalize();

    return 0;
}
