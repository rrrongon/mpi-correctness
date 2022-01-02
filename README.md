# mpi-correctness
MPI Routines correctness benchmarking

Compile:
mpicc -o sampleexecutable.exe correctness_file.c

Run:
mpiexec -n 7 -f hostfile ./sampleexecutable.exe DEBUG [Default 0]

DEBUG = 1 [ Show extended LOG for debugging]
DEBUG = 0 [ Show only PASS/FAIL]
