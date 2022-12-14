#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>
#include "ppm.h"

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time, setup_time, flip_time, gray_time, smooth_time, out_time, close_time; 
    double mpi1_start, mpi1_end, mpi2_start, mpi2_end;
    double time_s, time_p, speedup, effi;

    int i, j, k;
    char input_file[30], output_file[30];
    strcpy(input_file, argv[1]);
    strcpy(output_file, argv[2]);

    start_time = MPI_Wtime();

    // Create the Datatype for MPI
    MPI_Datatype rgbMPI;
    int lengths[3] = {1,1,1};
    MPI_Aint displacements[3];
    RGB dummy_rgb;
    MPI_Aint base_address;
    MPI_Get_address(&dummy_rgb, &base_address);
    MPI_Get_address(&dummy_rgb.R, &displacements[0]);
    MPI_Get_address(&dummy_rgb.G, &displacements[1]);
    MPI_Get_address(&dummy_rgb.B, &displacements[2]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
    
    MPI_Datatype types[3] = { MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR };
    MPI_Type_create_struct(3, lengths, displacements, types, &rgbMPI);
    MPI_Type_commit(&rgbMPI);

    PPMImage ori_ppm, result, tmp_ppm;
    // Read ppm image
    fnReadPPM(input_file, &ori_ppm);
    
    // Init result, received ppm image
    fnInitPPM(&result, &ori_ppm);
    fnInitPPM(&tmp_ppm, &ori_ppm);
    setup_time = MPI_Wtime();

    // Calculate own index range
    int h_start = (ori_ppm.height * rank) / size;
    int h_end = (ori_ppm.height * (rank + 1)) / size;

    // Flip an image horizontally
    for (i=h_start; i<h_end; i++) {
        for (j=0; j<ori_ppm.width; j++) {
            result.pixels[i*ori_ppm.width + j] = ori_ppm.pixels[i*ori_ppm.width + j];
            result.pixels[i*ori_ppm.width + result.width - j -1] = ori_ppm.pixels[i*ori_ppm.width + j];
        }
    }
    flip_time = MPI_Wtime();

    // reduce the image to grayscale
    for (i=h_start; i<h_end; i++) {
        for (j=0; j<ori_ppm.width; j++) {
            unsigned char avg = (result.pixels[i*ori_ppm.width + j].R 
                                + result.pixels[i*ori_ppm.width + j].G
                                + result.pixels[i*ori_ppm.width + j].B)/3;
            result.pixels[i*ori_ppm.width + j].R = avg;
            result.pixels[i*ori_ppm.width + j].G = avg;
            result.pixels[i*ori_ppm.width + j].B = avg;
        }
    }
    gray_time = MPI_Wtime();

    int len_height = (h_end - h_start) * ori_ppm.width;
    int counts[size];
    MPI_Allgather(&len_height, 1, MPI_INT, counts, 1, MPI_INT, MPI_COMM_WORLD);

    int displ[size];
    displ[0] = 0;
    for (i=1; i<size; i++) {
        displ[i] = displ[i-1] + counts[i-1];
    }

    // for sync
    MPI_Barrier(MPI_COMM_WORLD);
    mpi1_start = MPI_Wtime();
    MPI_Allgatherv(&result.pixels[h_start * ori_ppm.width], len_height, rgbMPI, tmp_ppm.pixels, counts, displ, rgbMPI, MPI_COMM_WORLD); 

    // For sync
    MPI_Barrier(MPI_COMM_WORLD);
    mpi1_end = MPI_Wtime();

    // Smooth the image 
    int dx[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

    for (i=h_start; i<h_end; i++) {
        for (j=0; j<ori_ppm.width; j++) {
            unsigned char value = 0;
            for (k=0; k<9; k++) {
                int ni = i+dx[k];
                int nj = j+dy[k];
                if(ni >= 0 && ni < ori_ppm.height && nj >= 0 && nj < ori_ppm.width) {
                    value += tmp_ppm.pixels[ni*ori_ppm.width + nj].R/9;
                    value += tmp_ppm.pixels[ni*ori_ppm.width + nj].G/9;
                    value += tmp_ppm.pixels[ni*ori_ppm.width + nj].B/9;
                }
            }
            tmp_ppm.pixels[i*ori_ppm.width + j].R = value;
            tmp_ppm.pixels[i*ori_ppm.width + j].G = value;
            tmp_ppm.pixels[i*ori_ppm.width + j].B = value;
        }
    }
    smooth_time = MPI_Wtime();

    
    // For sync
    MPI_Barrier(MPI_COMM_WORLD);
    mpi2_start = MPI_Wtime();
    MPI_Allgatherv(&tmp_ppm.pixels[h_start * ori_ppm.width], len_height, rgbMPI, result.pixels, counts, displ, rgbMPI, MPI_COMM_WORLD); 
    
    mpi2_end = MPI_Wtime();

    if (rank == 0) {
        fnWritePPM(output_file, &result);
    }
    out_time = MPI_Wtime();
    
    fnClosePPM(&ori_ppm);
    fnClosePPM(&result);
    fnClosePPM(&tmp_ppm);
    close_time = MPI_Wtime();

    if (rank == 0) {
        double setup, flip, gray, smooth, out, close, mpi1, mpi2;
        double serial, parallel;
        setup = setup_time - start_time;
        flip = flip_time - setup_time;
        gray = gray_time - flip_time;
        smooth = smooth_time - mpi1_end;
        out = out_time - mpi2_end;
        close = close_time - out_time;
        mpi1 = mpi1_end - mpi1_start;
        mpi2 = mpi2_end - mpi2_start;

        serial = setup + out + close;
        parallel = flip + gray + smooth + mpi1 + mpi2;
        time_s = serial + (parallel * size);
        time_p = serial + parallel;

        speedup = time_s/time_p;
        effi = speedup/size;
        
        printf("==============PARALLEL==============\n");
        printf("MPI size : %d\n", size);
        printf("Total execution time : %.6f\n", close_time - start_time);
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