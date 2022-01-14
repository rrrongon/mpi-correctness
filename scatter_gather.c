
#include <time.h>
#include <stdio.h>
#include  <mpi.h>
#include <math.h>
#include <stdlib.h>

float compute_average(float * sub_rand_nums, int num_elements);
float * generate_random_nums( int num_elements);

int main(int argc, char** argv){
    if (argc != 2){
        fprintf (stderr ,"Usage executable_program num_elements_per_proc\n");
        exit(1);
    }
    
    int num_elements_per_proc = atoi(argv[1]);
    
    srand(time(NULL));
    
    MPI_Init(NULL, NULL);
    
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    float * rand_nums;
    if(world_rank==0){
        rand_nums = generate_random_nums(num_elements_per_proc * world_size);
    }
    
    float * sub_rand_nums = (float *)malloc(num_elements_per_proc * sizeof(float));
    
    MPI_Scatter (rand_nums, num_elements_per_proc, MPI_FLOAT, sub_rand_nums, num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    float avg_val = compute_average(sub_rand_nums, num_elements_per_proc);
   
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf("processor %s, rank %d calculated avg %f\n,", processor_name, world_rank, avg_val);
 
    float *sub_avgs = NULL;
    if (world_rank == 0) {
        sub_avgs = (float *)malloc(sizeof(float) * world_size);
    }
    
    MPI_Gather(&avg_val, 1, MPI_FLOAT, sub_avgs, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    if (world_rank == 0) {
        float final_avg = compute_average(sub_avgs, world_size);
        
        printf("Avg of all elements is %f\n", final_avg);
        float original_data_avg = compute_average(rand_nums, num_elements_per_proc * world_size);
        printf("Avg computed across original data is %f\n", original_data_avg);
    }
    
    if (world_rank == 0) {
        free(rand_nums);
        free(sub_avgs);
    }
    free(sub_rand_nums);

    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Finalize();
    return 0;
}

float * generate_random_nums( int num_elements){
    float *rand_nums = (float *) malloc(sizeof(float) * num_elements);
    
    int i;
    for (i=0; i< num_elements; i++) {
        rand_nums[i] = (rand() / (float)RAND_MAX);
    }
    return rand_nums;
}

float compute_average(float * sub_rand_nums, int num_elements){
    float sum = 0.f;
    int i;
    
    for (i=0; i< num_elements; i++) {
        sum += sub_rand_nums[i];
    }

    float avg = sum / num_elements;

    return sum / num_elements ;
}

