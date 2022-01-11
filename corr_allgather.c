#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>


#define SIZES 10
/*
 *
 * Allgather: Root process generates data to process. Scatters to processes
 * Processes runs operation over those different input
 * send result to root
 * Root checks if calculation is correctly done
*/

/*Generating random numbers for input*/
float *create_rand_nums(int num_elements) {

        float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
        int i;

        for (i = 0; i < num_elements; i++) {
                rand_nums[i] = (rand() / (float)RAND_MAX);
        }

        return rand_nums;
}

/*Computing average for particular count of numbers*/
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


        int sizes[SIZES] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
        int size_counter;
        
        srand(time(NULL));

        MPI_Init(NULL, NULL);

        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

                /*
        *       Root generates input data to distribute task among all process
        *       sub_rand_nums to accept sub-task input from root
        *       */
        for (size_counter=0; size_counter<SIZES; size_counter++){
       
                int num_elements_per_proc = sizes[size_counter] * 1024;
                float *rand_nums = NULL;
                if (my_rank == 0) {
                        rand_nums = create_rand_nums(num_elements_per_proc * world_size);
                }

                float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);

                /*
        *       Each process call scatter routine to accept sub-input from root
        *       */

                MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

                /*Compute average assigned to each process*/
                float sub_avg = compute_avg(sub_rand_nums, num_elements_per_proc);

                /*Call Allgather routine from each process
        *       Need memory to receive avg from all other processes
        *       */      
                float *sub_avgs = (float *)malloc(sizeof(float) * world_size);
                MPI_Allgather(&sub_avg, 1, MPI_FLOAT, sub_avgs, 1, MPI_FLOAT, MPI_COMM_WORLD);

                float avg = compute_avg(sub_avgs, world_size);

                if(DEBUG_LOG)
                        printf("Avg of all elements from proc %d is %f\n", my_rank, avg);

                MPI_Barrier(MPI_COMM_WORLD);

                if (my_rank == 0) {
                        
                        /*
        *               Calculate all value average in root process to check final average received from sub-tasks
        *               */
                        float all_value_avg = compute_avg(rand_nums, num_elements_per_proc * world_size);

                        MPI_Status status;
                        int global_pass = 1;
                        float avg_recv = 0;
                        int local_pass = 1;
                        float diff = avg - all_value_avg;
                        /*Since average might be few points deviated from one process to another
        *               We can accept few decimals deviation from what rank 0 calculated and what was received from all process
        *               */
                        if (diff >0.001){
                                global_pass = 0;
                                if(DEBUG_LOG)
                                        printf(RED "FAIL: RANK 0. allgather: %f, calculated: %f\n", avg, all_value_avg );
                        }else{
                                if(DEBUG_LOG)
                                        printf("RANK 0: allgather: %f, calculated: %f\n", avg, all_value_avg);
                        }

                        /*
        *               Receive at rank 0 what other processes received by allgather routine.
        *               */
                        int rank;
                        for(rank=1; rank < world_size; rank++){

                                MPI_Recv(&avg_recv, 1, MPI_FLOAT, rank, 0, MPI_COMM_WORLD, &status);
                                diff = all_value_avg - avg_recv;
                                if (diff < 0)
                                        diff = diff * (-1);
                                if(diff > 0.001)
                                        local_pass = 0;
                                else
                                        local_pass = 1;

                                global_pass = global_pass && local_pass;

                                if(DEBUG_LOG){
                                        if(local_pass)
                                                printf("RANK %d: allgather: %f, Expected: %f\n", rank, avg_recv, all_value_avg);
                                        else
                                                printf(RED"RANK: %d FAIL. Allgather: %f, Expected: %f\n"RESET, rank, avg_recv, all_value_avg);
                                }                               

                        }

                        free(rand_nums);

                        if(global_pass)
                                printf("TEST %d*1024 byte: PASS\n", sizes[size_counter]);
                        else
                                printf("TEST %d*1024 byte: FAIL\n", sizes[size_counter]);

                }else{
                        /*Send calculated value received by routine to rank 0 to check*/
                        MPI_Send(&avg, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
                }

                free(sub_avgs);
                free(sub_rand_nums);

                MPI_Barrier(MPI_COMM_WORLD);
                
                MPI_Comm New_Comm;
                if(world_size > 4 ){
                        int color = my_rank % 2;
                        int new_id, new_world_size, broad_val;

                        MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
                        MPI_Comm_rank(New_Comm, &new_id);
                        MPI_Comm_size(New_Comm, &new_world_size);

                        /*
        *               each subcomm world might have different number of processes.
        *               */
                        float *rand_nums = NULL;
                        if (new_id == 0 && my_rank == 0) {
                                rand_nums = create_rand_nums(num_elements_per_proc * new_world_size);
                        }else if (new_id == 0 && my_rank != 0){
                                rand_nums = create_rand_nums(num_elements_per_proc * new_world_size);
                        }

                        float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);

                        /*
        *               Scatter sub-task among processes
        *               Complete subtask
        *               Each process call routine to collect subtasks from other
        *               */
                        MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, New_Comm);

                        float sub_avg = compute_avg(sub_rand_nums, num_elements_per_proc);
                        
                        float *sub_avgs = (float *)malloc(sizeof(float) * new_world_size);
                        MPI_Allgather(&sub_avg, 1, MPI_FLOAT, sub_avgs, 1, MPI_FLOAT, New_Comm);

                        float avg = compute_avg(sub_avgs, new_world_size);

                        if(DEBUG_LOG)
                        printf("SUBCOMM:  RANK %d,NEW RANK %d,SUB-TASK AVG: %f, TASK AVG: %f\n", my_rank, new_id,sub_avg, avg);

                        MPI_Barrier(New_Comm);
                        

                        int subcomm_pass;
                        if (new_id == 0) {

                                float all_value_avg = compute_avg(rand_nums, num_elements_per_proc * new_world_size);

                                MPI_Status status;
                                subcomm_pass = 1;
                                float avg_recv = 0;
                                int local_pass = 1;
                                float diff = avg - all_value_avg;

                                if (diff >0.001){
                                        subcomm_pass = 0;
                                        if(DEBUG_LOG)
                                                printf(RED"SUBCOM NEWID %d: FAIL. SIZE %d*1024. Routine Calculation=%f, EXPECTED=%f\n"RESET,new_id, sizes[size_counter], avg, all_value_avg );
                                }else{
                                        if(DEBUG_LOG)
                                                printf("SUBCOM NEWID %d: OK. SIZE %d*1024. Routine Calculation=%f, EXPECTED=%f\n",new_id, sizes[size_counter], avg, all_value_avg );
                                }
                                int rank;
                                for(rank=1; rank < new_world_size; rank++){

                                        MPI_Recv(&avg_recv, 1, MPI_FLOAT, rank, 0, New_Comm, &status);
                                        diff = all_value_avg - avg_recv;
                                        if (diff < 0)
                                                diff = diff * (-1);
                                        if(diff > 0.001)
                                                local_pass = 0;
                                        else
                                                local_pass = 1;

                                        subcomm_pass = subcomm_pass && local_pass;

                                        if(DEBUG_LOG)
                                                printf("NEWID %d: SIZE: %d*1024. Routine Calculation: %f, EXPECTED: %f. LOCAL STATUS: %d\n", rank, sizes[size_counter], avg_recv, all_value_avg, local_pass);
                                }

                                free(rand_nums);

                        }else{
                                //Send calculated average value to root
                                int cnt = 1;
                                int dest_rank = 0;
                                int tag = 0;
                                MPI_Send(&avg, cnt, MPI_FLOAT, dest_rank, tag, New_Comm);
                        }

                        /*
                *       One subcom root send decision to global root to make ultimate check
                *       */
                        if (my_rank ==0 && new_id == 0 ) {
                                MPI_Status status;
                                int recv_subcom_pass;
                                MPI_Recv(&recv_subcom_pass, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

                                if(recv_subcom_pass && subcomm_pass )
                                        printf("SUBCOMM TEST: PASS. SIZE: %d*1024\n", sizes[size_counter]);
                                else
                                        printf(RED"SUBCOMM TEST: FAIL. SIZE: %d*1024\n"RESET, sizes[size_counter]);

                        }else if(my_rank != 0 && new_id == 0) {
                                MPI_Send(&subcomm_pass, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                        }
                        MPI_Barrier(New_Comm);
                }
        }

        MPI_Finalize();

        return EXIT_SUCCESS;
}