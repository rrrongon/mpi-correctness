#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[])

{
    MPI_Init(&argc, &argv);

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //Integer test
    int root=0;

    int min_int = -9999999;
    int data_count = 1;
    MPI_Bcast(&min_int, data_count, MPI_INT, root, MPI_COMM_WORLD);	
    
    if(my_rank==0)
    {
	// Receive min value to check what it received.
	MPI_Status status;
	int min_val_recv;
	
	printf("INTEGER Test\n");
	bool passed = false;
	for(int i=1;i< size; i++)
	{
	    MPI_Recv(&min_val_recv, data_count, MPI_INT, i,1, MPI_COMM_WORLD, &status);
	    if(min_val_recv == min_int)
	    {
		//printf("RANK %d: PASSED\n", i);
		passed = true;
	    }else
	    {
		passed = false;
		break;
	    }
	}

	if (passed){
	    printf("Integer: PASSED\n");
	}else
	{
	    printf("Integer: FAILED\n");
	}
    }else
    {
	int min_val_recv;
	int data_count=1;
	MPI_Status status;
	
	int destination_rank=0;
	MPI_Send(&min_int, data_count, MPI_INT, destination_rank, 1, MPI_COMM_WORLD);	
    }

    //ARRAY test
    double test_array[5] = {1000.0, 2.0, 3.4, 7.0, 50.0};
    data_count = 5;
    MPI_Bcast(&test_array, data_count, MPI_DOUBLE, root, MPI_COMM_WORLD);
        
    if(my_rank==0)
    {
        MPI_Status status;
        double test_array_rec[5];

        printf("ARRAY Test\n");
        bool passed = false;
        for(int i=1;i< size; i++)
        {
            MPI_Recv(&test_array_rec, data_count, MPI_DOUBLE, i,1, MPI_COMM_WORLD, &status);
	    for(int j=0; j< 5; j++)
	    {
		if(test_array_rec[j]==test_array[j])
		    passed = true;
		else
		{
		    passed = false;
		    break;
		}
	    }
	    if(!passed)
	    {
		printf("RANK %d FAILED\n", i);
	    }
        }

        if (passed){
            printf("ARRAY: PASSED\n");
        }else
        {
            printf("ARRAY: FAILED\n");
        }
    }else
    {
        int data_count=5;
        MPI_Status status;

        int destination_rank=0;
        MPI_Send(&test_array, data_count, MPI_DOUBLE, destination_rank, 1, MPI_COMM_WORLD);
    }


    //Char test
    char test_string[1000] ="Creates new communicators based on colors and keys. Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys.Creates new communicators based on colors and keys";
    char rec_string[1000];    

    data_count = strlen(test_string);
    MPI_Bcast(&test_string, data_count, MPI_CHAR, root, MPI_COMM_WORLD);

    if(my_rank==0)
    {
        MPI_Status status;
        char test_string_rec[1000];

        printf("Char Test\n");
        bool passed = false;
        for(int i=1;i< size; i++)
        {
            MPI_Recv(&test_string_rec, data_count, MPI_CHAR, i,1, MPI_COMM_WORLD, &status);
            for(int j=0; j< data_count; j++)
            {
                if(test_string_rec[j]==test_string[j])
                    passed = true;
                else
                {
                    passed = false;
                    break;
                }
            }
            if(!passed)
            {
                printf("RANK %d FAILED\n", i);
            }
        }

        if (passed){
            printf("CHAR: PASSED\n");
        }else
        {
            printf("CHAR: FAILED\n");
        }
    }else
    {
        MPI_Status status;

        int destination_rank=0;
        MPI_Send(&test_string, data_count, MPI_CHAR, destination_rank, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
        
    return EXIT_SUCCESS;
 }
        
