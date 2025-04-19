#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "kernel.h"

float** createKernel(int size) {
	float** kernel = malloc(sizeof(float*) * size);
	int i;
	for(i = 0; i < size; i++) {
		kernel[i] = malloc(sizeof(float) * size);
	}
	return kernel;
}

void invertKernel(float** kernel, int size) {
	int i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			kernel[i][j] = -kernel[i][j];
		}
	}
}

float** createBoxBlurKernel(int size) {
	float** kernel = createKernel(size);
	float value = 1.0f / (size * size);
	int i, j;
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			kernel[i][j] = value;
		}
	}
	return kernel;
}

float** createGaussianBlurKernel(int size) {
	float** kernel = createKernel(size);
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

float** createSharpenKernel(int size) {
	int i, j;
	int center = size / 2;
	float** kernel = createKernel(size);
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			kernel[i][j] = -1;
		}
	}
	kernel[center][center] = size * size;
	return kernel;
}

float** createGaussianSharpenKernel(int size) {
	float** kernel = createGaussianBlurKernel(size);
	int center = size / 2;
	invertKernel(kernel, size);
	kernel[center][center] = 2;
	return kernel;
}

float** createOutlineKernel(int size) {
	float** kernel = createSharpenKernel(size);
	int center = size / 2;
	kernel[center][center] -= 1;
	return kernel;
}
