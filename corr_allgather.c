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
 *  *  *  *
 *   *  Allgather: Root process generates data to process. Scatters to processes
 *    *  Processes again call allgather routine to collect data in all processes
 *     *   Each process sends collected data to root
 *      *  Root checks if processes got correct data from routine
 *       *       *      */


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


        int sizes[SIZES] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
        int size_counter;
        
        srand(time(NULL));

        MPI_Init(NULL, NULL);

        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        /*
        Root generates input data to distribute task among all process
 	sub_rand_nums to accept sub-task input from root
       */
        for (size_counter=0; size_counter<SIZES; size_counter++){
       
                int num_elements_per_proc = sizes[size_counter] * 1024;
                float *rand_nums = NULL;
                if (my_rank == 0) {
                        rand_nums = create_rand_nums(num_elements_per_proc * world_size);
                }

                float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);

                /*
 *                  Each process call scatter routine to accept sub-input from root
 *              */

                MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);


                /*Call Allgather routine from each process
 *                  Need memory to receive avg from all other processes
 *                                   */      
                float *recv_buf = (float *)malloc(sizeof(float) * num_elements_per_proc * world_size);
                MPI_Allgather(sub_rand_nums, num_elements_per_proc, MPI_FLOAT, recv_buf, num_elements_per_proc, MPI_FLOAT, MPI_COMM_WORLD);

                if (my_rank == 0) {
                        
                        MPI_Status status;
                        int global_pass, local_pass, rank, data_cnt,pos;
			float v1,v2;
                        float *ranks_recv_buf = (float *)malloc(sizeof(float) * num_elements_per_proc * world_size);
                        global_pass=1;                        
                        data_cnt = num_elements_per_proc * world_size;
			
                        for(rank=1; rank < world_size; rank++){
				local_pass = 1;
				/*Collect allgather data from all processes to check what they received.
 * 				And check element by element*/
                                MPI_Recv(ranks_recv_buf, data_cnt, MPI_FLOAT, rank, 0, MPI_COMM_WORLD, &status);

                                for(pos=0; pos< data_cnt; pos++){
                                       	v1 = ranks_recv_buf[pos];
                                        v2 = rand_nums[pos];

                                        if(v1!=v2)
                                                local_pass = local_pass && 0;
                                        else
                                                local_pass = local_pass && 1;

				}				
                                global_pass = global_pass && local_pass;
				
				if(local_pass){
                                        if(DEBUG_LOG)
                                                printf("RANK %d: PASS\n", rank);
        
                                }else{
                                                printf(RED"RANK: %d FAIL\n"RESET, rank);
                                }

                        }

			/*Check for rank 0 too*/
			local_pass = 1;
                        for(pos=0; pos< data_cnt; pos++){
                                v1 = recv_buf[pos];
                                v2 = rand_nums[pos];

                                if(v1!=v2)
                                        local_pass = local_pass && 0;
                                else
                                        local_pass = local_pass && 1;

			}
                        
                        global_pass = global_pass && local_pass;
				
                        if(local_pass){
                                if(DEBUG_LOG)
                                        printf("RANK %d: PASS\n", my_rank);

                        }else{
                                        printf(RED"RANK: %d FAIL\n"RESET, my_rank);
                        }


                        free(rand_nums);
                        free(ranks_recv_buf);

                        if(global_pass)
                                printf("TEST %d*1024 byte: PASS\n", sizes[size_counter]);
                        else
                                printf(RED"TEST %d*1024 byte: FAIL\n"RESET, sizes[size_counter]);

                }else{
			/*Send value received by routine to rank 0 to check*/
			int data_cnt = num_elements_per_proc * world_size;
                        int tag = 0;
                        int dest_rank = 0;
                        MPI_Send(recv_buf, data_cnt, MPI_FLOAT, dest_rank, tag, MPI_COMM_WORLD);
                }


                free(recv_buf);
                free(sub_rand_nums);
    
                MPI_Barrier(MPI_COMM_WORLD);
                
                MPI_Comm New_Comm;
                if(world_size > 4){
                        int color = my_rank % 2;
                        int new_id, new_world_size, broad_val;

                        MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
                        MPI_Comm_rank(New_Comm, &new_id);
                        MPI_Comm_size(New_Comm, &new_world_size);

                        /*
                        each subcomm world might have different number of processes.
                       */
                        float *rand_nums = NULL;
                        if (new_id == 0) {
                                rand_nums = create_rand_nums(num_elements_per_proc * new_world_size);
                        }
                        float *sub_rand_nums = (float *)malloc(sizeof(float) * num_elements_per_proc);
                
                        MPI_Scatter(rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, New_Comm);

                        float *recv_buf = (float *)malloc(sizeof(float) * num_elements_per_proc * new_world_size);
                        MPI_Allgather(sub_rand_nums, num_elements_per_proc, MPI_FLOAT, recv_buf, num_elements_per_proc, MPI_FLOAT, New_Comm);

                        if(new_id==0){


                                MPI_Status status;
                                int sub_global_pass, sub_local_pass, rank, data_cnt,pos;
                                float v1,v2;
                                float *ranks_recv_buf = (float *)malloc(sizeof(float) * num_elements_per_proc * new_world_size);
                                sub_global_pass=1;                        
                                data_cnt = num_elements_per_proc * new_world_size;
			
                                for(rank=1; rank < new_world_size; rank++){
                                        sub_local_pass = 1;

                                        MPI_Recv(ranks_recv_buf, data_cnt, MPI_FLOAT, rank, 0, New_Comm, &status);

                                        for(pos=0; pos< data_cnt; pos++){
                                                v1 = ranks_recv_buf[pos];
                                                v2 = rand_nums[pos];

                                                if(v1!=v2)
                                                        sub_local_pass = sub_local_pass && 0;
                                                else
                                                        sub_local_pass = sub_local_pass && 1;

                                        }				
                                        sub_global_pass = sub_global_pass && sub_local_pass;
                                        
                                        if(sub_local_pass){
                                                if(DEBUG_LOG)
                                                        printf("NEWRANK %d: PASS\n", rank);
                
                                        }else{
                                                        printf(RED"NEWRANK: %d FAIL\n"RESET, rank);
                                        }

                                }
				
				sub_local_pass = 1;
                                for(pos=0; pos< data_cnt; pos++){
                                        v1 = recv_buf[pos];
                                        v2 = rand_nums[pos];

                                        if(v1!=v2)
                                                sub_local_pass = sub_local_pass && 0;
                                        else
                                                sub_local_pass = sub_local_pass && 1;

                                }
                                
                                sub_global_pass = sub_global_pass && sub_local_pass;
                                        
                                if(sub_local_pass){
                                        if(DEBUG_LOG)
                                                printf("NEW RANK %d: PASS\n", new_id);

                                }else{
                                                printf(RED"RANK: %d FAIL\n"RESET, new_id);
                                }

                                free(rand_nums);
                                free(ranks_recv_buf);

                                if(sub_global_pass)
                                        printf("SUBCOMM TEST %d*1024 byte: PASS\n", sizes[size_counter]);
                                else
                                        printf(RED"SUBCOMM TEST %d*1024 byte: FAIL\n"RESET, sizes[size_counter]);

				/*One of 2 new subcom roots, will send it work decision to global root to make final verdict*/	
                                if(my_rank == 0){
                                        
                                        MPI_Status status;
			                int recv_subcom_pass;
			                MPI_Recv(&recv_subcom_pass, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

                                        if(recv_subcom_pass && sub_global_pass )
                                                printf("SUBCOMM TEST: PASS\n");
                                        else
                                                printf(RED"SUBCOMM TEST: FAIL \n"RESET);
                                }else{
                                        MPI_Send(&sub_global_pass, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                                }

                        }else{
                                int data_cnt = num_elements_per_proc * new_world_size;
                                int tag = 0;
                                int dest_rank = 0;
                                MPI_Send(recv_buf, data_cnt, MPI_FLOAT, dest_rank, tag, New_Comm);
                        }
			MPI_Barrier(New_Comm);
                }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();

        return EXIT_SUCCESS;
}

