#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]){

	MPI_Init(&argc, &argv);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/*MPI_Group world_group, customGrp1, customGrp2;

	MPI_Comm customCom1, customCom2;

	int half_world_size = world_size/2 ;
	int *custom_proc_rank = (int*) malloc(half_world_size * sizeof(int));
	
	for(int i=0 ;i< half_world_size; i++)
		custom_proc_rank[i] = i;

	MPI_Comm_group(MPI_COMM_WORLD, &world_group);
	
	MPI_Group_incl(world_group, half_world_size, custom_proc_rank, &customGrp1);
	MPI_Comm_create(MPI_COMM_WORLD, customGrp1, &customCom1);

	int val = 100;
	MPI_Bcast(&val, 1, MPI_INT, 0, customCom1);	

	if(my_rank==1){	
	for(int i=1; i<half_world_size ;i++){
		if(my_rank==i){
			int rec_val;
			MPI_Status status;
			MPI_Recv(&rec_val, 1, MPI_INT, 0,0, customCom1, &status);
			printf("data received RANK %d and value %d\n", my_rank, rec_val);
		}
	}
	}*/

	int color = my_rank % 2;
	MPI_Comm New_Comm;
	int new_id, new_world_size, broad_val;

	MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &New_Comm);
	MPI_Comm_rank(New_Comm, &new_id);
	MPI_Comm_size(New_Comm, &new_world_size);

	if(new_id == 0) 
		broad_val = color;
 
	MPI_Bcast(&broad_val, 1, MPI_INT, 0, New_Comm);
 
	printf("Old_proc[%d] has new rank %d received value  %d\n", my_rank, new_id, broad_val);

	MPI_Finalize();
        
    	return EXIT_SUCCESS;
}
