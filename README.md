# EXPLORER Lab Correctness Testing Suite for MPI Routines

This testing suite checks the correctess of different implementations of the following mpi operations:
- p2p (Point to point send/recv operations)
- Allgather
- Allreduce
- Alltoall
- Bcast
- Reduce
- Scatter




## Compile:
Set the path to the mpi compiler (mpicc) in the first line of the provided Makefile.
Then, "make" command will compile all test programs. If you wish to compile the test of a specific operator, you need to provide the target name from Makefile in your make command. 

e. g.: make allgather => will only compuile the test for allgather


## Run:
mpiexec -n 7 -f hostfile ./sampleexecutable.exe DEBUG [Default 0]

DEBUG = 1 [ Show extended LOG for debugging]
DEBUG = 0 [ Show only PASS/FAIL]
