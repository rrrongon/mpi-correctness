#include </nfsdata/rongon/summer-21/mpi/mpi_installation/include/mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
//initialize mpi block
	MPI_Init(NULL,NULL);

//find current process rank and group process size
	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	int world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_size);

// pass some value from one process to other
	int passing_value = 0;
	int ping_count = 0;
	int destination_rank = (my_rank+1)%2;

	while (ping_count < 10){
		if (my_rank == ping_count%2){
	// Send value to other process (data address, how many, data_type, destination rank, tag, communicator)
			passing_value = -1;
			int count=1;
			ping_count++;
			MPI_Send(&ping_count, count, MPI_INT, destination_rank, 0, MPI_COMM_WORLD);
				
		}else{
			MPI_Recv ( &ping_count, 1, MPI_INT, destination_rank ,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			printf("Process %d received ping  %d from process %d\n", my_rank, ping_count, destination_rank ) ;
		}
	}

	MPI_Finalize();
}
