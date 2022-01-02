#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m" 
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]){
    
	int DEBUG_LOG =0; 
	if(argv[1]==NULL)
                DEBUG_LOG = 0;
        else
                DEBUG_LOG = atoi(argv[1]);

	MPI_Init(NULL,NULL);

    	int size;
    	MPI_Comm_size(MPI_COMM_WORLD, &size);
    	int my_rank;
    	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    	int root=0;
	/*Broadcast some large int value*/
    	int min_int;
        if(my_rank==0) 
                min_int = -9999999;
    	int data_count = 1;
    	MPI_Bcast(&min_int, data_count, MPI_INT, root, MPI_COMM_WORLD);	
    	
        int global_pass = 1;
    
    	if(my_rank==0){
		MPI_Status status;
		int min_val_recv;
	
		bool passed = true;
		/*Ask from all process what they received via broadcast routine
 *		Check if received value is equal to what was sent by root
 * 		*/
		for(int i=1;i< size; i++){
	    		MPI_Recv(&min_val_recv, data_count, MPI_INT, i,1, MPI_COMM_WORLD, &status);
	    		if(min_val_recv == min_int){
				passed = passed && true;
	    		}else{
				passed = passed && false;
	    		}

			if(passed){
				if(DEBUG_LOG)
					printf("TEST: INT; PASS. RANK %d\n",i);
			}else{
				printf(RED"TEST: FAIL. RANK %d. EXPECTED: %d, RECEIVED: %d\n"RESET, i,min_int,min_val_recv);
			}
		}

		global_pass = global_pass && passed;

    	}else{
		/*Send received value via routine to root for cross-match*/
		int min_val_recv;
		int data_count=1;
		MPI_Status status;
	
	        int destination_rank=0;
	        MPI_Send(&min_int, data_count, MPI_INT, destination_rank, 1, MPI_COMM_WORLD);	
    	}

    	//Array test
	float * test_array = (float *) malloc( sizeof(float) * 5);
        if(my_rank==0){
		for(int i=0;i<5;i++)
			test_array[i] = i * 100;
	} 

    	data_count = 5;
    	MPI_Bcast(test_array, data_count, MPI_FLOAT, root, MPI_COMM_WORLD);
 
   	if(my_rank==0){
        	MPI_Status status;
        	float * test_array_rec = (float *) malloc( sizeof(float) * 5);

        	bool passed = true;
        
		for(int i=1;i< size; i++){
            		MPI_Recv(test_array_rec, data_count, MPI_FLOAT, i,1, MPI_COMM_WORLD, &status);
	    		for(int j=0; j< 5; j++){
				if(test_array_rec[j]==test_array[j])
		    			passed = passed && true;
				else{
		    			passed = passed && false;
		    	
				}
	    		}
	    
			if(!passed)
				printf(RED "TEST: ARRAY. RANK %d FAILED\n", i);

			if(passed && DEBUG_LOG)
				printf("TEST: ARRAY PASS. RANK %d\n", i);
	    
        	}

        	global_pass = global_pass && passed;

    	}else{
        	int data_count=5;
        	MPI_Status status;

        	int destination_rank=0;
        	MPI_Send(test_array, data_count, MPI_FLOAT, destination_rank, 1, MPI_COMM_WORLD);
    	}

	char *test_string= (char *)malloc(sizeof(char) * 1000);;
	if(my_rank==0)
    		test_string="Creates new communicators based on colors and keys. Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys";
    	char *rec_string = (char *)malloc(sizeof(char) * 1000);;    

    	data_count = strlen(test_string);
    	MPI_Bcast(test_string, 1000, MPI_CHAR, root, MPI_COMM_WORLD);

    	if(my_rank==0){
        	MPI_Status status;
        	char * test_string_rec = (char *)malloc(sizeof(char) * 1000);

        	bool passed = true;
        	for(int i=1;i< size; i++){
            		MPI_Recv(test_string_rec, 1000, MPI_CHAR, i,1, MPI_COMM_WORLD, &status);
            		for(int j=0; j< data_count; j++){
                		if(test_string_rec[j]==test_string[j])
                    			passed = passed && true;
                		else{
                    			passed = passed && false;
                		}
            		}
            
			if(!passed)
                                printf(RED "TEST: CHAR. RANK %d FAILED\n", i);

                        if(passed && DEBUG_LOG)
                                printf("TEST: CHAR PASS. RANK %d\n", i); 
        	}

        	global_pass = global_pass && passed;

    	}else{
        	MPI_Status status;

        	int destination_rank=0;
        	MPI_Send(test_string,1000, MPI_CHAR, destination_rank, 1, MPI_COMM_WORLD);
    	}

    	if(my_rank == 0){
        	if(global_pass)
                    	printf("TEST: PASS\n");
           	 else
                    printf(RED"TEST: FAIL\n"RESET);
    	}

        if(size > 4){
		//Create New world
                int color = my_rank % 2;
                MPI_Comm New_Comm;
                int new_id, new_world_size, broad_val;

                MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
                MPI_Comm_rank(New_Comm, &new_id);
                MPI_Comm_size(New_Comm, &new_world_size);

                int subcomm_min_int;

                if(new_id == 0) 
                        subcomm_min_int = -9999999;
                int data_count = 1;
                MPI_Bcast(&subcomm_min_int, data_count, MPI_INT, 0, New_Comm);	
                
                int subcomm_final_pass = 1;
                int subcomm_global_pass = 1;
            
                if(new_id==0){
                        MPI_Status status;
                        int subcomm_min_val_recv;
            
                        int subcomm_passed = 1;

                        for(int i=1;i< new_world_size; i++){
                                MPI_Recv(&subcomm_min_val_recv, data_count, MPI_INT, i,1, New_Comm, &status);
                                if(subcomm_min_val_recv == subcomm_min_int){
                                        subcomm_passed = subcomm_passed && 1;
                                }else{
                                        subcomm_passed = subcomm_passed && 0;
                                }

				if(subcomm_passed){
                                	if(DEBUG_LOG)
                                        	printf("TEST: INT; PASS. RANK %d\n", i);
                        	}else{
                                	printf(RED"TEST: FAIL. NEWRANK %d. EXPECTED: %d, RECEIVED: %d\n"RESET, i,subcomm_min_int, subcomm_min_val_recv);
                        	}
                        }
                        
                        subcomm_global_pass = subcomm_global_pass && subcomm_passed;
			
			/*Share inter subcomm-root decision*/
                        if(my_rank==0){
                                MPI_Status temp_status;
                                int subcomm_global_pass_recv;
                                MPI_Recv(&subcomm_global_pass_recv, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &temp_status);

                                subcomm_final_pass = subcomm_final_pass && subcomm_global_pass_recv;

				if(DEBUG_LOG){
					if(subcomm_final_pass)
						printf("SUBCOMM TEST: INT PASS");
					else
						printf(RED"SUBCOMM TEST: INT FAIL" RESET);
				}		
                        }else{
                                MPI_Send(&subcomm_global_pass, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
                        }

                }else{
                        int subcomm_min_val_recv;
                        int data_count=1;
            
                        int destination_rank=0;
                        MPI_Send(&subcomm_min_int, data_count, MPI_INT, destination_rank, 1, New_Comm);	
                }

		/*Generate input data in root to broadcast*/
                float * subcomm_test_array = (float *)malloc(sizeof(float) * 5);;
                if(new_id==0)
			for(int i=0;i<5;i++)
                        	subcomm_test_array[i] = i*100; 
                data_count = 5;
                MPI_Bcast(subcomm_test_array, data_count, MPI_FLOAT, 0, New_Comm);

                if(new_id==0){
                        MPI_Status status;
                        float * subcomm_test_array_rec =  (float *)malloc(sizeof(float) * 5);;

                        int subcomm_passed = 1;
                    
                        for(int i=1;i< new_world_size; i++){
				int local_pass = 1;
				int data_cnt = 5;
				int tag = 1;
                                MPI_Recv(subcomm_test_array_rec, data_cnt, MPI_FLOAT, i, tag, New_Comm, &status);
                                for(int j=0; j< 5; j++){
                                        if(subcomm_test_array_rec[j]==subcomm_test_array[j]){
                                                subcomm_passed = subcomm_passed && 1;
						local_pass = local_pass && 1;
                                        }else{
                                                subcomm_passed = subcomm_passed && 0;
						local_pass = local_pass && 0;
                                        }

                                }

				if(DEBUG_LOG){
                                	if(local_pass)
                                        	printf("TEST ARRAY: NEWRANK %d PASS.\n", i);
                                        else
                                        	printf(RED"TEST ARRAY: NEWRANK %d FAIL\n" RESET, i);
                                }
                        
                        }

                        subcomm_global_pass = subcomm_global_pass && subcomm_passed;

                        if(my_rank==0){
                                MPI_Status temp_status;
                                int subcomm_global_pass_recv;
                                MPI_Recv(&subcomm_global_pass_recv, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &temp_status);

                                subcomm_final_pass = subcomm_final_pass && subcomm_global_pass_recv;

				if(DEBUG_LOG){
					if(subcomm_final_pass)
						printf("SUBCOMM TEST ARRAY: PASS\n");
					else
						printf(RED"SUBCOMM TEST ARRAY: FAIL\n"RESET);
				}
				
                        }else{
                                MPI_Send(&subcomm_global_pass, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
                        }

                }else{
                	int data_count=5;
                    	MPI_Status status;

                    	int destination_rank=0;
                    	MPI_Send(subcomm_test_array, data_count, MPI_FLOAT, destination_rank, 1, New_Comm);
                }

                char * subcomm_test_string =  (char *)malloc(sizeof(char) * 1000);;
                if(new_id==0)
                	subcomm_test_string ="Creates new communicators based on colors and keys. Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys";  

                data_count = 1000;
                MPI_Bcast(subcomm_test_string, data_count, MPI_CHAR, 0, New_Comm);

                if(new_id==0){
                        MPI_Status status;
                        char * subcomm_test_string_rec =  (char *)malloc(sizeof(char) * 1000);

                        int subcomm_passed = 1;
                        for(int i=1;i< new_world_size; i++){
				int local_pass = 1;
                                MPI_Recv(subcomm_test_string_rec, 1000, MPI_CHAR, i,1, New_Comm, &status);
                                for(int j=0; j< 1000; j++){
                                    if(subcomm_test_string_rec[j]==subcomm_test_string[j]){
                                            subcomm_passed = subcomm_passed && 1;
						local_pass = local_pass && 1;
                                    }else{
                                            subcomm_passed = subcomm_passed && 0;
						local_pass = local_pass && 0;
                                    }
                                }
                       		
				if(DEBUG_LOG){
                                        if(local_pass)
                                                printf("TEST CHAR: NEWRANK %d PASS.\n", i);
                                        else
                                                printf(RED"TEST CHAR: NEWRANK %d FAIL\n" RESET, i);
                                } 
                        }
 
                        subcomm_global_pass = subcomm_global_pass && subcomm_passed;
			
			/*Inter subcomm root decision share*/
                        if(my_rank==0){
                                MPI_Status temp_status;
                                int subcomm_global_pass_recv;
                                MPI_Recv(&subcomm_global_pass_recv, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &temp_status);

                                subcomm_final_pass = subcomm_final_pass && subcomm_global_pass_recv;
				
				if(subcomm_final_pass)
                                	printf("SUBCOMM TEST: PASS\n");
                        	else
                                	printf(RED"SUBCOMM TEST: FAIL\n"RESET);

                        }else{
                                MPI_Send(&subcomm_global_pass, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
                        }

                }else{
                        int destination_rank=0;
                        MPI_Send(subcomm_test_string, 1000, MPI_CHAR, destination_rank, 1, New_Comm);
                }
        }

    	MPI_Finalize();
        
    	return EXIT_SUCCESS;
 }
