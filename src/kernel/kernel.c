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
	float sigma = size / 1.8f;
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

float** create_emboss_kernel(int size, int direction) {
	float** kernel = create_kernel(size);
	int center = size / 2;
	int i, j, di, dj;
	int flipper_left_right = (direction == 0 || direction == 2) ? 1 : -1;
	int flipper_up_down = (direction == 0 || direction == 1) ? 1 : -1;

	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			di = (float)(i - center);
			dj = (float)(j - center);

			if(di == flipper_up_down * dj) {
				kernel[i][j] = (di > 0) ? flipper_left_right * 2.0f : flipper_left_right * -2.0f;
			} else if(di == 0 || dj == 0) {
				kernel[i][j] = (di + (flipper_up_down * dj) > 0) ? flipper_left_right * 1.0f : flipper_left_right *  -1.0f;
			} else {
				kernel[i][j] = 0.0f;
			}
		}
	}

	kernel[center][center] = 1.0f;

	return kernel;
}

float** create_sobel_kernel(int size, int direction) {
	float** kernel = create_kernel(size);
	int center = size / 2;
	int i, j, di, dj;
	float gx_val, gy_val, final_val;

	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			di = i - center;
			dj = j - center;

			gx_val = (di * ((dj == 0) ? 2.0f : 1.0f));
			gy_val = (dj * ((di == 0) ? 2.0f : 1.0f));

			switch(direction) {
				case 0:
					final_val = -gx_val;
					break;
				case 1:
					final_val = gy_val;
					break;
				case 2:
					final_val = gx_val;
					break;
				case 3:
					final_val = -gy_val;
					break;
			}
			kernel[i][j] = final_val;
		}
	}

	return kernel;
}
