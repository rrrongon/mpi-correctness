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
        DEBUG_LOG = atoi(argv[1]);

        int num_elements_per_proc = 1000;//atoi(argv[1]);

        srand(time(NULL));

        MPI_Init(NULL, NULL);

        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        float *rand_nums = NULL;
        if (my_rank == 0) {
                rand_nums = create_rand_nums(num_elements_per_proc * world_size);
        }

        float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);


        MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

        float sub_avg = compute_avg(sub_rand_nums, num_elements_per_proc);

        float *sub_avgs = (float *)malloc(sizeof(float) * world_size);
        MPI_Allgather(&sub_avg, 1, MPI_FLOAT, sub_avgs, 1, MPI_FLOAT, MPI_COMM_WORLD);

        float avg = compute_avg(sub_avgs, world_size);

        if(DEBUG_LOG)
                printf("Avg of all elements from proc %d is %f\n", my_rank, avg);

        MPI_Barrier(MPI_COMM_WORLD);

        if (my_rank == 0) {

                float all_value_avg = compute_avg(rand_nums, num_elements_per_proc * world_size);

                MPI_Status status;
                int global_pass = 1;
                float avg_recv = 0;
                int local_pass = 1;
                float diff = avg - all_value_avg;

                if (diff >0.001){
                        global_pass = 0;
                        if(DEBUG_LOG)
                                printf("avg did not match. allgather avg=%f, calculated avg=%f\n", avg, all_value_avg );
                }else{
                        if(DEBUG_LOG)
                                printf("avg is EQUAL. allgather avg=%f, calculated avg=%f\n", avg, all_value_avg);
                }

                for(int rank=1; rank < world_size; rank++){

                        MPI_Recv(&avg_recv, 1, MPI_FLOAT, rank, 0, MPI_COMM_WORLD, &status);
                        diff = all_value_avg - avg_recv;
                        if (diff < 0)
                                diff = diff * (-1);
                        if(diff > 0.001)
                                local_pass = 0;
                        else
                                local_pass = 1;

                        global_pass = global_pass && local_pass;

                }

                free(rand_nums);

                if(global_pass)
                        printf("TEST: PASS\n");
                else
                        printf("TEST: FAIL\n");

        }else{
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

                float *rand_nums = NULL;
                if (new_id == 0 && my_rank == 0) {
                    rand_nums = create_rand_nums(num_elements_per_proc * new_world_size);
                }else if (new_id == 0 && my_rank != 0){
                    rand_nums = create_rand_nums(num_elements_per_proc * new_world_size);
                }

                float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);

                MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, New_Comm);

                float sub_avg = compute_avg(sub_rand_nums, num_elements_per_proc);

                float *sub_avgs = (float *)malloc(sizeof(float) * new_world_size);
                MPI_Allgather(&sub_avg, 1, MPI_FLOAT, sub_avgs, 1, MPI_FLOAT, New_Comm);

                float avg = compute_avg(sub_avgs, new_world_size);

                if(DEBUG_LOG)
                    printf("SUBCOMM: Avg of all elements from proc %d is %f\n", new_id, avg);

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
                                        printf("avg did not match. allgather avg=%f, calculated avg=%f\n", avg, all_value_avg );
                        }else{
                                if(DEBUG_LOG)
                                        printf("avg is EQUAL. allgather avg=%f, calculated avg=%f\n", avg, all_value_avg);
                        }

                        for(int rank=1; rank < new_world_size; rank++){

                                MPI_Recv(&avg_recv, 1, MPI_FLOAT, rank, 0, New_Comm, &status);
                                diff = all_value_avg - avg_recv;
                                if (diff < 0)
                                        diff = diff * (-1);
                                if(diff > 0.001)
                                        local_pass = 0;
                                else
                                        local_pass = 1;

                                subcomm_pass = subcomm_pass && local_pass;


                        }

                    free(rand_nums);

                }else{
                    MPI_Send(&avg, 1, MPI_FLOAT, 0, 0, New_Comm);
                }

                if (my_rank ==0 && new_id == 0 ) {
                        MPI_Status status;
			            int recv_subcom_pass;
			            MPI_Recv(&recv_subcom_pass, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

                        if(recv_subcom_pass && subcomm_pass )
                                printf("SUBCOMM TEST: PASS\n");
                        else
                                printf("SUBCOMM TEST: FAIL \n");

                }else if(my_rank != 0 && new_id == 0) {
                    MPI_Send(&subcomm_pass, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
            MPI_Barrier(New_Comm);
        }
        
        MPI_Finalize();

        return EXIT_SUCCESS;
}

