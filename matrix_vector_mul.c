#include <stdio.h>
#include  "/nfsdata/ProgramFiles/ompi-3.1.8/include/mpi.h"
#include <math.h>
#include <stdlib.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int MAX_ROWS = 1000;
int MAX_COLS = 1000;

int main(int argc, char *argv[]){
	int rows, cols;
	double ans, a[MAX_ROWS][MAX_COLS], b[MAX_COLS], c[MAX_ROWS], buffer[MAX_COLS];
	
	int myid, manager, numprocs;
	int i,j, numsent, sender, anstype, row;
	MPI_Status status;

	MPI_Init(&argc,&argv);
 	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
 	printf("Number of processes %d\n", numprocs);
 	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	manager = 0;
	rows = 100;
	cols = 100;

	if (myid == 0){
		for (j=1;j<=cols;j++){
			b[j]=1;
			for (i=1;i<=rows;i++){
				a[i][j] = i;
			}
		}

		numsent = 0;
		MPI_Bcast(&b, cols, MPI_DOUBLE, manager, MPI_COMM_WORLD);

		// send rows to each worker tagging with rownumber
		for (i=1;i<= MIN(numprocs-1, rows); i++){
			for (j=1; j < cols;j++){
				buffer[j] = a[i][j];
			}
			MPI_Send(&buffer,cols,MPI_DOUBLE, i, i, MPI_COMM_WORLD);
		}

		// receive
		for (i=1;i<=rows; i++){
			MPI_Recv(&ans,1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			sender = status.MPI_SOURCE;
			anstype = status.MPI_TAG;
			c[anstype] = ans;
	
			if (numsent < rows){
				for (j=1; j < cols;j++){
                                	buffer[j] = a[i][j];
                        	}
                        	MPI_Send(&buffer,cols,MPI_DOUBLE, i, i, MPI_COMM_WORLD);

				numsent = numsent + 1;
			}else {
				MPI_Send(MPI_BOTTOM,0,MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
			}
		}

		for (i=1;i<=cols;i++)
			printf("c(%d) = %lf", i,c[i]);
	} else {
		MPI_Bcast(&b, cols, MPI_DOUBLE, manager, MPI_COMM_WORLD);
		if (myid < rows){
			MPI_Recv(&buffer, cols, MPI_DOUBLE, manager, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if (status.MPI_TAG==0)
				exit(1);

			row = status.MPI_TAG;
			ans = 0.0;
			for (i=1;i<cols;i++)
				ans = ans + buffer[i]* b[i];
			
			MPI_Send(&ans,1,MPI_DOUBLE, manager,row,MPI_COMM_WORLD);
		}
	}
	MPI_Finalize();	
	return 0;
}
