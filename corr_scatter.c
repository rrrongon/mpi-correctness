#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

float *create_rand_nums(int num_elements) {
  float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
  int i;
  for (i = 0; i < num_elements; i++) {
    rand_nums[i] = (rand() / (float)RAND_MAX);
  }
  return rand_nums;
}


float compute_avg(float *array, int num_elements) {
  float sum = 0.f;
  int i;
  for (i = 0; i < num_elements; i++) {
    sum += array[i];
  }
  return sum / num_elements;
}


int main(int argc, char** argv) {

	int DEBUG_LOG =0; 
	if(argv[1]==NULL)
                DEBUG_LOG = 0;
        else
                DEBUG_LOG = atoi(argv[1]);

	  int num_elements_per_proc = 1000;

  	srand(time(NULL));

  	MPI_Init(NULL, NULL);

  	int my_rank;
  	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  	int world_size;
  	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	/*Root process generates input for every process to scatter*/
  	float *rand_nums = NULL;
  	if (my_rank == 0) {
    		rand_nums = create_rand_nums(num_elements_per_proc * world_size);
  	}

	//subtask receive buffer disbursed by root
  	float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);


  	MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums,
              num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

	//complete subtask
  	float sub_avg = compute_avg(sub_rand_nums, num_elements_per_proc);
	if (DEBUG_LOG)
		printf("RANK %d Calculated: %f\n", my_rank, sub_avg);

	//Send completed subtask result to root
  	if(my_rank!=0){
    		int data_count = 1;
    		int destination_rank = 0;
    		int TAG = 1;
    		MPI_Send(&sub_avg, data_count, MPI_FLOAT, destination_rank, TAG, MPI_COMM_WORLD);
  	}

	if (my_rank == 0) {    
    		float test_avg_float;
    		float temp_sum=0;
    		MPI_Status status;
    		bool passed = false;

		//Calculate portion of random number should process X receive. Calculate that portion sum to check
		int i;
    		for(i=1; i< world_size; i++){
        		temp_sum = 0;
			int j;
			for(j= i*1000; j < ((i+1)*1000) ; j++){
	  			temp_sum = temp_sum + rand_nums[j]; 
			}
	
			float temp_avg = temp_sum/1000;

			MPI_Recv(&test_avg_float, 1, MPI_FLOAT, i,1, MPI_COMM_WORLD, &status);
			
			if(DEBUG_LOG)
				printf("RANK %d Calculated %f, Expected: %f\n", my_rank, temp_avg, test_avg_float);
			
			if(temp_avg == test_avg_float){
	    			passed = true;
			}else{
	    			passed = false;
	    			if(DEBUG_LOG)
					printf(RED"TEST: FAIL RANK %d"RESET, i);
        		}
    		}

    		if(passed)
        		printf("TEST: PASSED\n");
    		else
			printf("TEST: FAILED\n");
		
    		free(rand_nums);
  }

    	if(world_size > 4){
        	int color = my_rank % 2;
        	MPI_Comm New_Comm;
        	int new_id, new_world_size, broad_val;

        	MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
        	MPI_Comm_rank(New_Comm, &new_id);
        	MPI_Comm_size(New_Comm, &new_world_size);

        	float *subcomm_rand_nums = NULL;
        	if (new_id == 0) {
            		subcomm_rand_nums = create_rand_nums(num_elements_per_proc * new_world_size);
        	}

        	float *subcomm_local_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);
    
        	MPI_Scatter(subcomm_rand_nums, num_elements_per_proc, MPI_FLOAT, subcomm_local_rand_nums,
              	num_elements_per_proc, MPI_FLOAT, 0, New_Comm);

        	float subcomm_local_avg = compute_avg(subcomm_local_rand_nums, num_elements_per_proc);
		if(DEBUG_LOG)
			printf("SUbCOMM SUM: %f, NEWRANK %d\n", subcomm_local_avg, new_id);		

		//Send calculated avg to root
        	if(new_id!=0){
            		int data_count = 1;
            		int destination_rank = 0;
            		int TAG = 1;
            		MPI_Send(&subcomm_local_avg, data_count, MPI_FLOAT, destination_rank, TAG, New_Comm);
        	}

        	if (new_id == 0) {    
            		float test_avg_float;
            		float temp_sum=0;
            		MPI_Status status;
            		int passed = 1;

			int i;
            		for(i=1; i< new_world_size; i++){
                		temp_sum = 0;
				int j;
                		for(j= i*1000; j < ((i+1)*1000) ; j++){
                    			temp_sum = temp_sum + subcomm_rand_nums[j]; 
                		}
                		float temp_avg = temp_sum/1000;

                		MPI_Recv(&test_avg_float, 1, MPI_FLOAT, i,1, New_Comm, &status);

				if(DEBUG_LOG)
					printf("RANK %d NEWRANK %d Calculated %f, Expected: %f\n", my_rank, new_id, test_avg_float, temp_avg)
;
                		if(temp_avg == test_avg_float){
                    			passed = passed && 1;
                		}else{
                    			passed = passed && 0;
                    			printf(RED"SUBCOMM: FAIL RANK %d NEWRANK %d. CALCULATED: %f, Routine: %f\n"RESET, my_rank, new_id, temp_avg, test_avg_float);
                		}
            		}

			/*Inter subcomm root decision exchange for final check*/
            		if(my_rank==0){
                		int peer_pass;
                		MPI_Status temp_status;
                		MPI_Recv(&peer_pass, 1, MPI_INT, 1,2, MPI_COMM_WORLD, &temp_status);

                		passed = passed && peer_pass;

                		if(passed)
                    			printf("SUBCOM TEST: PASS\n");
                		else
                    			printf(RED"SUBCOM TEST: FAIL\n"RESET);

            		}else{
                		int destination_rank = 0;
                		int TAG = 2;
                		MPI_Send(&passed, 1, MPI_INT, destination_rank, TAG, MPI_COMM_WORLD);
            		}

         	   	free(subcomm_local_rand_nums);
            		free(subcomm_rand_nums);
        	}

    	}


  	free(sub_rand_nums);

  	MPI_Barrier(MPI_COMM_WORLD);
  	MPI_Finalize();
}
