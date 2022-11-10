#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time, gen_time, mpi_time; 
    double time_s, time_p, speedup, effi;

    start_time = MPI_Wtime();
    // generating random integer
    srand(time(NULL));
    int r = rand()%1000;
    gen_time = MPI_Wtime();
    
    int total; 
    MPI_Scan(&r, &total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    mpi_time = MPI_Wtime();

    if (rank == 0) {
        double gen, mpi;
        gen = gen_time - start_time;
        mpi = mpi_time - gen_time;

        time_s = gen + (mpi * size);
        time_p = gen + mpi;

        speedup = time_s/time_p;
        effi = speedup/size;
        
        printf("==============MPI_SCAN==============\n");
        printf("MPI size : %d\n", size);
        printf("Total execution time : %.6f\n", mpi_time - start_time);
        printf("Ts = %.6f\n", time_s);
        printf("Tp = %.6f\n", time_p);
        printf("Scaled SpeedUp = %.6f\n", speedup);
        printf("Efficiency : %.6f\n", effi);
        printf("====================================\n");
        printf("\n");
    }
    
    MPI_Finalize();

    return 0;
}