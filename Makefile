MPICC := $(shell which mpicc)
all: allgather allreduce alltoall bcast reduce scatter p2p gather

allgather: 
	$(MPICC) -o allgather_correctness.exe corr_allgather.c

allreduce: corr_allreduce.c
	$(MPICC) -o allreduce_correctness.exe corr_allreduce.c -lm

alltoall: corr_alltoall.c
	$(MPICC) -o alltoall_correctness.exe corr_alltoall.c

bcast: corr_bcast.c
	$(MPICC) -o bcast_correctness.exe corr_bcast.c -lm

reduce: corr_reduce.c
	$(MPICC) -o reduce_correctness.exe corr_reduce.c

scatter: corr_scatter.c
	$(MPICC) -o scatter_correctness.exe corr_scatter.c

p2p: corr_p2p.c
	$(MPICC) -o p2p_correctness.exe corr_p2p.c -lm

gather: corr_gather.c
	$(MPICC) -o gather_correctness.exe corr_gather.c -lm

clean:
	rm -f *.x *.o *.exe
