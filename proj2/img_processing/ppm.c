#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppm.h"

#define TRUE 1
#define FALSE 0

// #define PPMREADBUFLEN 256

void fnInitPPM(PPMImage* target,PPMImage* img) {
	int i,j;
    target->height = img->height;
    target->width = img->width;

	target->pixels = (RGB**)calloc(img->height, sizeof(RGB*));
	for(i=0; i<img->height; i++){
	   target->pixels[i] = (RGB*)calloc(img->width, sizeof(RGB));
	}

	for(i = 0 ; i < img->height ; i++)
	{
		for(j = 0 ; j < img->width ;j++)
		{
			target->pixels[i][j].R = 0;
			target->pixels[i][j].G = 0;
			target->pixels[i][j].B = 0;
		}
	}
}

int fnReadPPM(char* fileNm, PPMImage* img)
{
	FILE* fp;

	if(fileNm == NULL){
		fprintf(stderr, "fnReadPPM 호출 에러\n");
		return FALSE;
	}
	
	fp = fopen(fileNm, "rb");	// binary mode
	if(fp == NULL){
		fprintf(stderr, "파일을 열 수 없습니다 : %s\n", fileNm);
		return FALSE;
	}

	fscanf(fp, "%c%c\n", &img->M, &img->N);	// 매직넘버 읽기

	if(img->M != 'P' || img->N != '6'){
		fprintf(stderr, "PPM 이미지 포멧이 아닙니다 : %c%c\n", img->M, img->N);
		return FALSE;
	}

    // char buf[PPMREADBUFLEN], *t;
    // do
	// { /* Px formats can have # comments after first line */
	// 	t = fgets(buf, PPMREADBUFLEN, fp);
	// 	if ( t == NULL ) return FALSE;
	// } while ( strncmp(buf, "#", 1) == 0 );

	fscanf(fp, "%d %d\n", &img->width, &img->height);	// 가로, 세로 읽기
	fscanf(fp, "%d\n"   , &img->max                );	// 최대명암도 값

	if(img->max != 255){
		fprintf(stderr, "올바른 이미지 포멧이 아닙니다.\n");
		return FALSE;
	}


	// <-- 메모리 할당
	img->pixels = (RGB**)calloc(img->height, sizeof(RGB*));

	for(int i=0; i<img->height; i++){
	   // 1개의 픽셀을 위해 R, G, B 3byte가 필요
	   img->pixels[i] = (RGB*)calloc(img->width, sizeof(RGB));
	}
	// -->


	// <-- ppm 파일로부터 픽셀값을 읽어서 할당한 메모리에 load
	for(int i=0; i<img->height; i++){
		for(int j=0; j<img->width; j++){
			// fread(&img->pixels[i][j], sizeof(unsigned char), 1, fp);
            fread(&img->pixels[i][j].R, sizeof(unsigned char), 1, fp);
            fread(&img->pixels[i][j].G, sizeof(unsigned char), 1, fp);
            fread(&img->pixels[i][j].B, sizeof(unsigned char), 1, fp);
		}
	}
	// -->


	fclose(fp);	// 더 이상 사용하지 않는 파일을 닫아 줌

	return TRUE;
}

int fnWritePPM(char* fileNm, PPMImage* img)
{
	FILE* fp;

	fp = fopen(fileNm, "wb");
	if(fp == NULL){
		fprintf(stderr, "파일 생성에 실패하였습니다.\n");
		return FALSE;
	}

	fprintf(fp, "%c%c\n", 'P', '6');
	fprintf(fp, "%d %d\n" , img->width, img->height);
	fprintf(fp, "%d\n", 255);

	for(int i=0; i<img->height; i++){
		for(int j=0; j<img->width; j++){
            fwrite(&img->pixels[i][j], sizeof(unsigned char), 3, fp);
            // fwrite(&img->pixels[i][j].R, sizeof(unsigned char), 1, fp);
            // fwrite(&img->pixels[i][j].G, sizeof(unsigned char), 1, fp);
            // fwrite(&img->pixels[i][j].B, sizeof(unsigned char), 1, fp);

			// fprintf(fp, "%d ", img->pixels[i][j].R);
			// fprintf(fp, "%d ", img->pixels[i][j].G);
			// fprintf(fp, "%d ", img->pixels[i][j].B);
		}
		// fprintf(fp, "\n");	// 생략가능
	}

	fclose(fp);
	
	return TRUE;
}

void fnClosePPM(PPMImage* img) {
     free(img->pixels);
}

