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

	if (my_rank == 0){
// Send value to other process (data address, how many, data_type, destination rank, tag, communicator)
		passing_value = -1;
		int destination_rank = 1;
		int count=1;
		MPI_Send(&passing_value, count, MPI_INT, destination_rank, 0, MPI_COMM_WORLD);
	
	}else if (my_rank !=0){
		MPI_Recv ( &passing_value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		printf("Process %d received number %d from process 0\n", my_rank, passing_value) ;
	}

	MPI_Finalize();
}
