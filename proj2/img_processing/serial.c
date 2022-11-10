#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "ppm.h"

// argv[1] : input image name
// argv[2] : output image name
int main(int argc, char *argv[]) {
    struct timeval start, end;
    int i, j, k;
    char input_file[30], output_file[30];
    strcpy(input_file, argv[1]);
    strcpy(output_file, argv[2]);

    gettimeofday(&start, NULL);

    PPMImage ori_ppm, result;
    fnReadPPM(input_file, &ori_ppm);
    
    // Init result ppm image
    fnInitPPM(&result, &ori_ppm);

    // Flip an image horizontally
    for (i=0; i<ori_ppm.height; i++) {
        for (j=0; j<ori_ppm.width/2; j++) {
            // result.pixels[i][j].R = ori_ppm.pixels[i][j].R;
            // result.pixels[i][j].G = ori_ppm.pixels[i][j].G;
            // result.pixels[i][j].B = ori_ppm.pixels[i][j].B;
            
            // result.pixels[i][result.width - j -1].R = ori_ppm.pixels[i][j].R;
            // result.pixels[i][result.width - j -1].G = ori_ppm.pixels[i][j].G;
            // result.pixels[i][result.width - j -1].B = ori_ppm.pixels[i][j].B;
            result.pixels[i*ori_ppm.width + j] = ori_ppm.pixels[i*ori_ppm.width + j];
            result.pixels[i*ori_ppm.width + result.width - j -1] = ori_ppm.pixels[i*ori_ppm.width + j];
        }
    }

    fnWritePPM("output/flip.ppm", &result);

    // reduce the image to grayscale
    for (i=0; i<ori_ppm.height; i++) {
        for (j=0; j<ori_ppm.width; j++) {
            unsigned char avg = (result.pixels[i*ori_ppm.width + j].R
                                + result.pixels[i*ori_ppm.width + j].G
                                + result.pixels[i*ori_ppm.width + j].B)/3;
            result.pixels[i*ori_ppm.width + j].R = avg;
            result.pixels[i*ori_ppm.width + j].G = avg;
            result.pixels[i*ori_ppm.width + j].B = avg;
        }
    }

    fnWritePPM("output/reduce.ppm", &result);

    // smooth the image 
    int dx[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

    for (i=0; i<ori_ppm.height; i++) {
        for (j=0; j<ori_ppm.width; j++) {
            unsigned char value = 0;
            for (k=0; k<9; k++) {
                int ni = i+dx[k];
                int nj = j+dy[k];
                if(ni >= 0 && ni < ori_ppm.height && nj >= 0 && nj < ori_ppm.width) {
                    value += result.pixels[ni*ori_ppm.width + nj].R/9;
                    value += result.pixels[ni*ori_ppm.width + nj].G/9;
                    value += result.pixels[ni*ori_ppm.width + nj].B/9;
                }
            }
            result.pixels[i*ori_ppm.width + j].R = value;
            result.pixels[i*ori_ppm.width + j].G = value;
            result.pixels[i*ori_ppm.width + j].B = value;
        }
    }

    fnWritePPM(output_file, &result);

    fnClosePPM(&ori_ppm);
    fnClosePPM(&result);

    gettimeofday(&end, NULL);

    double totaltime = (((end.tv_usec - start.tv_usec) / 1.0e6 + end.tv_sec - start.tv_sec) * 1000) / 1000;
    printf("Total execution time = %f seconds\n", totaltime);

    return 0;
}