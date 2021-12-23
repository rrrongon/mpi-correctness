#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
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

        int num_elements_per_proc = 1000; //atoi(argv[1]);

        MPI_Init(NULL, NULL);

        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);


        srand(time(NULL)*my_rank); // Seed the random number generator of processes uniquely
        float *rand_nums = NULL;
        rand_nums = create_rand_nums(num_elements_per_proc);

        float local_sum = 0;
        int i;
        for (i = 0; i < num_elements_per_proc; i++) {
                local_sum += rand_nums[i];
        }

        float global_sum;
        MPI_Allreduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

        if (my_rank==0){
                float global_sum_recv = 0;
                MPI_Status status;

                bool passed = false;

                for (int rank=1; rank < world_size; rank++){
                        MPI_Recv(&global_sum_recv, 1, MPI_FLOAT, rank, 1, MPI_COMM_WORLD, &status);
                        if (global_sum_recv == global_sum){
                                passed = true;
                        }else{
                                passed = false;
                                break;
                        }
                }

                if(passed)
                        printf("PASSED\n");
                else
                        printf("FAILED\n");
        }else{
                MPI_Send(&global_sum, 1, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
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
                for (int i = 0; i < num_elements_per_proc; i++) {
                        subcom_local_sum += subcom_rand_nums[i];
                }

                float subcom_global_sum;
                int root = 0;

                int subcom_global_pass = 1;

                MPI_Allreduce(&subcom_local_sum, &subcom_global_sum, 1, MPI_FLOAT, MPI_SUM, New_Comm);

                if(new_id == 0 ){
                        MPI_Status subcom_status;
                        float subcom_cal_global_sum=0;
                        for(int rank=1; rank< new_world_size; rank++){
                                float subcom_recv_num;
                                
                                MPI_Recv (&subcom_recv_num, 1, MPI_FLOAT, rank, 1, New_Comm, &subcom_status);
                                subcom_cal_global_sum = subcom_cal_global_sum +  subcom_recv_num;
                        }

                        subcom_cal_global_sum = subcom_cal_global_sum + subcom_local_sum;

                        for(int rank=1; rank< new_world_size; rank++){
                                float subcom_recv_num;
                                
                                MPI_Recv (&subcom_recv_num, 1, MPI_FLOAT, rank, 2, New_Comm, &subcom_status);
                                
                                float diff = subcom_cal_global_sum - subcom_recv_num;

                                if(diff <0)
                                    diff = diff * (-1);

                                if(diff>0.001)
                                        subcom_global_pass = subcom_global_pass && 0;
                                else
                                        subcom_global_pass = subcom_global_pass && 1;
                        }

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
