#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include  "/nfsdata/ProgramFiles/ompi-3.1.8/include/mpi.h"

using namespace std;

int main(int argc, char *argv[]);

void timestamp();

int main(int argc, char *argv[])
{
	double *a;
	double *a_row;
	double ans;
	double *b;
  	int dest;
  	int dummy;
  	int i;
  	int id;
  	int ierr;
  	int j;
  	int j_one;
  	int k;
  	int m;
  	static int master = 0;
  	int n;
  	int num_rows;
  	int num_workers;
  	int p;
  	double r8_pi = 3.141592653589793;
  	MPI_Status status;
  	int tag;
  	int tag_done;
  	double *x;


	ierr = MPI_Init(&argc, &argv);

	if (ierr!=0)
	{
		cout << "\n MAT-VEC-MUL: returned error: "<< ierr << endl;
		exit(1);
	}

	ierr = MPI_Comm_rank (MPI_COMM_WORLD, &id );

	ierr = MPI_Comm_size(MPI_COMM_WORLD, &p);

	// If manager process then rank or id = 0
	if (id ==0){
		timestamp ( );
    		cout << "\n";
    		cout << "MATVEC - Master process:\n";
    		cout << "  C++ version\n";
    		cout << "  An MPI example program to compute\n";
    		cout << "  a matrix-vector product b = A * x.\n";
    		cout << "\n";
    		cout << "  Compiled on " << __DATE__ " at " << __TIME__ "\n";
    		cout << "\n";
    		cout << "  The number of processes is " << p << ".\n";
	}

	cout << "\n";
	cout << "Process " << id << "is active.\n";
	
	m = 100;
	n = 50;

	tag_done = m+1;

	if (id == 0)
	{
		cout << "Row count" << m << endl;
		cout << "Column Count" << n << endl;
	}

    // Initialize Matrix and vector
    if (id == 0) {
        a = new double[m*n];
        x = new double[n];
        b = new double[m];
        
        k = 0;
        
        for (i=1; i<=m; i++) {
            for (j=1; j<=n; j++) {
                a[k] = sqrt ( 2.0 / ( double ) ( n + 1 ) )
                          * sin ( ( double ) ( i * j ) * r8_pi / ( double ) ( n + 1 ) );
                
                k = k + 1;
            }
        }
        
        j_one = 17;
        
        for ( i = 0; i < n; i++ ){
          x[i] = sqrt ( 2.0 / ( double ) ( n + 1 ) )
            * sin ( ( double ) ( ( i + 1 ) * j_one ) * r8_pi / ( double ) ( n + 1 ) );
        }
        
        cout << "\n";
        cout << "MATVEC - Master process:\n";
        cout << "  Vector x\n";
        cout << "\n";
        for ( i = 0; i < n; i++ ){
          cout << setw(6) << i << "  "
               << setw(10) << x[i] << "\n";
        }
    }else{
        a_row = new double[n];
        x = new double[n];
    }
	
    //  Process 0 broadcasts the vector X to the other processes.
    ierr = MPI_Bcast ( x, n, MPI_DOUBLE, master, MPI_COMM_WORLD );

    if (id==0) {
        num_rows = 0;
        for (i=1; i<=p-1; i++) {
            dest = i;
            tag = num_rows;
            k = num_rows * n;

            ierr = MPI_Send ( a+k, n, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD );

            num_rows = num_rows + 1;
        }
        
        num_workers = p-1 ;
        
        for (; ; ) {
            ierr = MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            tag = status.MPI_TAG;
            b[tag] = ans;
            
            if (num_rows < m){
                num_rows = num_rows + 1;
                dest = status.MPI_SOURCE;
                tag = num_rows;
                k = num_rows * n;
                
                ierr = MPI_Send(a+k, n, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
            } else {
                num_workers = num_workers - 1;
                dummy = 0;
                dest = status.MPI_SOURCE;
                tag = tag_done;

                ierr = MPI_Send ( &dummy, 1, MPI_INT, dest, tag, MPI_COMM_WORLD );

                if ( num_workers == 0 ){
                    cout << "  Process " << id << " shutting down.\n";
                    break;
                }
            }
        }
        
        delete [] a;
        delete [] x;
    } else {
        for ( ; ; ){
              ierr = MPI_Recv ( a_row, n, MPI_DOUBLE, master, MPI_ANY_TAG,
                MPI_COMM_WORLD, &status );

              tag = status.MPI_TAG;

              if ( tag == tag_done )
              {
                cout << "  Process " << id << " shutting down.\n";
                break;
              }

              ans = 0.0;
              for ( i = 0; i < n; i++ )
              {
                ans = ans + a_row[i] * x[i];
              }

              ierr = MPI_Send ( &ans, 1, MPI_DOUBLE, master, tag, MPI_COMM_WORLD );
        }
        delete [] a_row;
        delete [] x;
    }

    if ( id == 0 )
      {
        cout << "\n";
        cout << "MATVEC - Master process:\n";
        cout << "  Product vector b = A * x\n";
        cout << "  (Should be zero, except for a 1 in entry " << j_one-1 << "\n";
        cout << "\n";
        for ( i = 0; i < m; i++ )
        {
          cout << setw(4) << i << "  "
               << setw(10) << b[i] << "\n";
        }

        delete [] b;
      }
    
    MPI_Finalize ( );
    //
    //  Terminate.
    //
      if ( id == 0 )
      {
        cout << "\n";
        cout << "MATVEC - Master process:\n";
        cout << "  Normal end of execution.\n";
        cout << "\n";
        timestamp ( );
      }
      return 0;
}

void timestamp()
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct std::tm *tm_ptr;
  std::time_t now;

  now = std::time ( NULL );
  tm_ptr = std::localtime ( &now );

  std::strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm_ptr );

  std::cout << time_buffer << "\n";

  return;
# undef TIME_SIZE
}
