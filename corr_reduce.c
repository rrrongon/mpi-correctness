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

float *create_rand_nums(int num_elements) {
	float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
	int i;
	for (i = 0; i < num_elements; i++) {
    		rand_nums[i] = (rand() / (float)RAND_MAX);
  	}
  	return rand_nums;
}

int main(int argc, char** argv) {

	int DEBUG_LOG =0; 
	if(argv[1]==NULL)
                DEBUG_LOG = 0;
        else
                DEBUG_LOG = atoi(argv[1]);

	int num_elements_per_proc = 1000;

   	MPI_Init(NULL, NULL);

    	int my_rank;
    	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    	int world_size;
    	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	/*Generate subtask of random number in each process
 * 	Calculate sum in each process*/
    	srand(time(NULL)*my_rank);
    	float *rand_nums = NULL;
    	rand_nums = create_rand_nums(num_elements_per_proc);

    	float local_sum = 0;
	int i;
    	for (i = 0; i < num_elements_per_proc; i++) {
        	local_sum += rand_nums[i];
    	}

	if(DEBUG_LOG)
		printf("SUM in RANK %d: %f\n", my_rank, local_sum);

	// Call routine and get sum in root 
    	float global_sum;
    	int root = 0;

        int global_pass = 0;

    	MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

        if(my_rank ==0 ){
        	MPI_Status status;
                float cal_global_sum=0;
		int rank;
                for(rank=1; rank< world_size; rank++){
                        float *recv_nums = (float *)malloc(sizeof(float) * 1000);;
                        float cal_local_sum = 0;
                        MPI_Recv (recv_nums, 1000, MPI_FLOAT, rank, 1, MPI_COMM_WORLD, &status);
			int j;
                        for (j=0; j< 1000; j++){
                                cal_local_sum = cal_local_sum + recv_nums[j];
                        }
                        cal_global_sum = cal_global_sum +  cal_local_sum;
                }

                cal_global_sum = cal_global_sum + local_sum;

		if(DEBUG_LOG)
			printf("Calculated SUM: %f, Routine SUM: %f\n", cal_global_sum, global_sum);

                float diff = global_sum - cal_global_sum;
                if(diff <0)
                        diff = diff * (-1);

                if(diff>0.001)
                        global_pass = 0;
                else
                        global_pass = 1;

                if(global_pass)
                        printf("TEST: PASS\n");
		else	
			printf(RED"TEST: FAIL\n"RESET);
        }else{
                MPI_Send(rand_nums, num_elements_per_proc , MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
        }

	MPI_Comm New_Comm;
        if(world_size >= 4){
        	int color = my_rank % 2;
            	int new_id, new_world_size, broad_val;

        	MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
        	MPI_Comm_rank(New_Comm, &new_id);
        	MPI_Comm_size(New_Comm, &new_world_size);

                srand(time(NULL)*new_id);
                float *subcom_rand_nums = NULL;
                subcom_rand_nums = create_rand_nums(num_elements_per_proc);

                float subcom_local_sum = 0;
		int i;
                for (i = 0; i < num_elements_per_proc; i++) {
                    	subcom_local_sum += subcom_rand_nums[i];
                }

		if(DEBUG_LOG)
			printf("SUM in NEWRANK %d: %f\n", new_id, subcom_local_sum);

                float subcom_global_sum;
                int root = 0;

                int subcom_global_pass = 0;

                MPI_Reduce(&subcom_local_sum, &subcom_global_sum, 1, MPI_FLOAT, MPI_SUM, 0, New_Comm);

                if(new_id == 0 ){
                        MPI_Status subcom_status;
                        float subcom_cal_global_sum=0;
			int rank;
                        for(rank=1; rank< new_world_size; rank++){
                                float *subcom_recv_nums = (float *)malloc(sizeof(float) * 1000);;
                                float subcom_cal_local_sum = 0;
                                MPI_Recv (subcom_recv_nums, 1000, MPI_FLOAT, rank, 1, New_Comm, &subcom_status);

				int j;
                                for (j=0; j< 1000; j++){
                                        subcom_cal_local_sum = subcom_cal_local_sum + subcom_recv_nums[j];
                                }
                                subcom_cal_global_sum = subcom_cal_global_sum +  subcom_cal_local_sum;
                        }

                        subcom_cal_global_sum = subcom_cal_global_sum + subcom_local_sum;
			if(DEBUG_LOG)
				printf("SUM Calculated SUBCOMM ROOT: %f and SUM by ROutine: %f\n", subcom_cal_global_sum, subcom_global_sum);

                        float diff = subcom_global_sum - subcom_cal_global_sum;
                        if(diff <0)
                                diff = diff * (-1);

                        if(diff>0.001)
                                subcom_global_pass = 0;
                        else
                                subcom_global_pass = 1;

			/*Inter subcomm root decision exchange*/
                        if(my_rank ==0){
                                int subcom_global_pass_recv;
                                MPI_Recv (&subcom_global_pass_recv, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &subcom_status);

                                if(subcom_global_pass && subcom_global_pass_recv)
                                        printf("SUBCOMM TEST: PASS\n");
                                else
                                        printf(RED"SUBCOMM TEST: FAIL\n"RESET);
                        }else{
                                MPI_Send(&subcom_global_pass, 1 , MPI_INT, 0, 1, MPI_COMM_WORLD);
                        }
                }else{
                    MPI_Send(subcom_rand_nums, num_elements_per_proc , MPI_FLOAT, 0, 1, New_Comm);
                }

                MPI_Barrier(New_Comm);
        }

    free(rand_nums);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}

