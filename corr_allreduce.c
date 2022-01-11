#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <stdbool.h>

/*
 *    Allreduce: Each process has subtask. They compute.
 *    Each process calls allreduce routine mentioning an operation
 *    Finally all process gets summed result
 *
 *    To check correctness: 
 *    Each process generates data. Sums
 *    Send generated data to root.
 *    Root collect all input data and sum to get Expected Reference sum
 *    Then processes also send what sum they received from Routine
 *    Root checks if calculation is correctly done by routine and sent to processes
 *      */

/*Generating random numbers for input*/
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

        int num_elements_per_proc = 1000; //atoi(argv[1]);

        MPI_Init(NULL, NULL);

        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	/*GEnerate input for each process*/
        srand(time(NULL)*my_rank); // Seed the random number generator of processes uniquely
        float *rand_nums = NULL;
        rand_nums = create_rand_nums(num_elements_per_proc);

	//completes subtask
        float local_sum = 0;
        int i;
        for (i = 0; i < num_elements_per_proc; i++) {
                local_sum += rand_nums[i];
        }

	//Call Routine and Exchange subtask to allreduce among all processes
        float global_sum;
        MPI_Allreduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

        if (my_rank==0){
                float global_sum_recv = 0;
                MPI_Status status;
		
		/*Receives sub-task input data from all process to calculate Final sum for reference*/
		int tag = 3;
		float sum=0;
		int cnt = num_elements_per_proc, rank;
		for(rank=1; rank < world_size; rank++){
			float *input_recv_buf = (float *)malloc(sizeof(float) * num_elements_per_proc);
			MPI_Recv(input_recv_buf, cnt, MPI_FLOAT, rank, tag , MPI_COMM_WORLD, &status);
                        int j;
			for ( j=0;j < cnt;j++)
				sum += input_recv_buf[j];

			if(DEBUG_LOG)
				printf("Received input from RANK %d in Root\n", rank);
		}
		// add rank 0's own sum
		sum += local_sum;
		if(DEBUG_LOG)
			printf("CALCULATED SUM:%f\n", sum);

		/*
 * 		Ask Process What they got from allreduce routine
 *		Check routine sum with Expected sum with a 0.01 margin deviation
 * 		*/
		
                bool passed = true;
		tag = 1;
		cnt = 1;
                for ( rank=1; rank < world_size; rank++){
                        MPI_Recv(&global_sum_recv, cnt, MPI_FLOAT, rank, tag, MPI_COMM_WORLD, &status);
			if(DEBUG_LOG)
				printf("RANK %d: ROUTINE SUM: %f, EXPECTED SUM %f\n ", rank, global_sum_recv, sum);

			float diff = global_sum_recv - sum;
			if(diff < 0)
				diff = diff * (-1);
			if(diff < 0.01){
                                passed = passed && true;
                        }else{
                                passed = passed && false;
                                if(DEBUG_LOG)
					printf(RED"TEST: FAIL. RANK %d\n"RESET, rank);
                        }
                }
		/*Check for rank 0 too*/
		float diff = global_sum_recv - sum;
                if(diff < 0)
                	diff = diff * (-1);

                if(passed && (diff < 0.01))
                        printf("TEST: PASS\n");
                else
                        printf(RED"TEST: FAIL\n"RESET);
        }else{
		/*
 *		Send input and Routine sum to rank 0 for checking
 * 		*/
		int dest_rank = 0;
		int tag = 3;
		MPI_Send(rand_nums, num_elements_per_proc, MPI_FLOAT, dest_rank, tag, MPI_COMM_WORLD);
		
		int cnt=1;
		tag = 1;
                MPI_Send(&global_sum,cnt , MPI_FLOAT, dest_rank, tag, MPI_COMM_WORLD);
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
                for ( i = 0; i < num_elements_per_proc; i++) {
                        subcom_local_sum += subcom_rand_nums[i];
                }

                float subcom_global_sum;
                int root = 0;

                int subcom_global_pass = 1;

                MPI_Allreduce(&subcom_local_sum, &subcom_global_sum, 1, MPI_FLOAT, MPI_SUM, New_Comm);

                if(new_id == 0 ){
                        MPI_Status subcom_status;
                        float subcom_cal_global_sum=0;
                        int rank;
                        for( rank=1; rank< new_world_size; rank++){
                                float subcom_recv_num;
                                
                                MPI_Recv (&subcom_recv_num, 1, MPI_FLOAT, rank, 1, New_Comm, &subcom_status);
                                subcom_cal_global_sum = subcom_cal_global_sum +  subcom_recv_num;
                        }

                        subcom_cal_global_sum = subcom_cal_global_sum + subcom_local_sum;
                        
                        for( rank=1; rank< new_world_size; rank++){
                                float subcom_recv_num;
                                
                                MPI_Recv (&subcom_recv_num, 1, MPI_FLOAT, rank, 2, New_Comm, &subcom_status);
                                
                                float diff = subcom_cal_global_sum - subcom_recv_num;

                                if(diff <0)
                                    diff = diff * (-1);

                                if(diff>0.001){
                                        subcom_global_pass = subcom_global_pass && 0;
					if(DEBUG_LOG)
						printf(RED"SUBCOMM TEST: FAIL. NEWID:%d EXPECTED: %f, Routine: %f\n"RESET, rank, subcom_cal_global_sum, subcom_recv_num);
                                }else{
                                        subcom_global_pass = subcom_global_pass && 1;
				}
                        }

			/*
 *			Finally non-zero rank in COMMWORLD sends sub-decision to True root
 * 			*/
                        if(my_rank ==0){
                                int subcom_global_pass_recv;
                                MPI_Recv (&subcom_global_pass_recv, 1, MPI_INT, 1, 3, MPI_COMM_WORLD, &subcom_status);

                                if(subcom_global_pass && subcom_global_pass_recv)
                                        printf("SUBCOMM TEST: PASS\n");
                                else
                                        printf("SUBCOMM TEST: FAIL\n");
                        }else{
                                MPI_Send(&subcom_global_pass, 1 , MPI_INT, 0, 3, MPI_COMM_WORLD);
                        }
                }else{
                    MPI_Send(&subcom_local_sum, 1 , MPI_FLOAT, 0, 1, New_Comm);
                    MPI_Send(&subcom_global_sum, 1 , MPI_FLOAT, 0, 2, New_Comm);
                }

                MPI_Barrier(New_Comm);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();

}
