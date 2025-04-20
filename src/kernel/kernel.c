#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "kernel.h"

float** create_kernel(int size) {
	float** kernel = malloc(sizeof(float*) * size);
	int i;
	for(i = 0; i < size; i++) {
		kernel[i] = malloc(sizeof(float) * size);
	}
	return kernel;
}

void free_kernel(float** kernel, int size) {
	int i;
	for(i = 0; i < size; i++) {
		free(kernel[i]);
	}
	free(kernel);
}

void invert_kernel(float** kernel, int size) {
	int i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			kernel[i][j] = -kernel[i][j];
		}
	}
}

float** create_box_blur_kernel(int size) {
	float** kernel = create_kernel(size);
	float value = 1.0f / (size * size);
	int i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			kernel[i][j] = value;
		}
	}
	return kernel;
}

float** create_guassian_blur_kernel(int size) {
	float** kernel = create_kernel(size);
	float sum = 0.0f;
	int center = size / 2;
	int i, j, di, dj;
	float value;
	float sigma = size / (size / 1.5);
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			di = i - center;
			dj = j - center;
			value = exp(-(di * di + dj * dj) / (2.0f * sigma * sigma));
			kernel[i][j] = value;
			sum += value;
		}
	}
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			if(kernel[i][j]) {
				kernel[i][j] /= sum;
			}
		}
	}
	return kernel;
}

float** create_sharpen_kernel(int size) {
	int i, j;
	int center = size / 2;
	float** kernel = create_kernel(size);
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			kernel[i][j] = -1;
		}
	}
	kernel[center][center] = size * size;
	return kernel;
}

float** create_guassian_sharpen_kernel(int size) {
	float** kernel = create_guassian_blur_kernel(size);
	int center = size / 2;
	invert_kernel(kernel, size);
	kernel[center][center] = 2;
	return kernel;
}

float** create_outline_kernel(int size) {
	float** kernel = create_sharpen_kernel(size);
	int center = size / 2;
	kernel[center][center] -= 1;
	return kernel;
}
