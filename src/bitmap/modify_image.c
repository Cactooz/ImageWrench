#include <stdint.h>
#include <stdlib.h>
#include "modify_image.h"
#include "write_image.h"
#include "read_image.h"
#include "../menu/menu.h"
#include "../kernel/kernel.h"

void apply_kernel(Image* image, Menu kernel_type, int kernel_size) {
	float** kernel;
	uint32_t** pixels;
	uint32_t** result;
	uint32_t height, width, i, j, ki, kj, ni, nj;
	uint8_t a, r, g, b;
	float a_sum, r_sum, g_sum, b_sum;
	int half_kernel = kernel_size / 2;

	switch(kernel_type) {
		case BLUR:
			kernel = create_box_blur_kernel(kernel_size);
			break;
		case SHARPEN:
			kernel = create_sharpen_kernel(kernel_size);
			break;
		case OUTLINE:
			kernel = create_outline_kernel(kernel_size);
			break;
		default:
			fprintf(stderr, "Error: Invalid kernel.");
			exit(10);
	}

	if(image->flags[INFO_HEADER]) {
		height = image->bm_info_header.height;
		width = image->bm_info_header.width;
	} else {
		height = image->bm_core_header.height;
		width = image->bm_core_header.width;
	}

	pixels = pixel_data_to_array(image);

	result = malloc(height* sizeof(uint32_t*));
	for (i = 0; i < height; i++) {
		result[i] = malloc(width* sizeof(uint32_t));
	}

	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			r_sum = 0.0f;
			g_sum = 0.0f;
			b_sum = 0.0f;
			a_sum = 0.0f;

			for (ki = 0; ki < kernel_size; ki++) {
				for (kj = 0; kj < kernel_size; kj++) {
					ni = i + ki - half_kernel;
					nj = j + kj - half_kernel;

					if(ni >= 0 && ni < height && nj >= 0 && nj < width) {
						a = (pixels[ni][nj] >> 24) & 0xFF;
						r = (pixels[ni][nj] >> 16) & 0xFF;
						g = (pixels[ni][nj] >> 8) & 0xFF;
						b = pixels[ni][nj] & 0xFF;

						a_sum += a * kernel[ki][kj];
						r_sum += r * kernel[ki][kj];
						g_sum += g * kernel[ki][kj];
						b_sum += b * kernel[ki][kj];
					}
				}
			}
			a = (a_sum < 0) ? 0 : ((a_sum > 255) ? 255 : (uint8_t) a_sum);
			r = (r_sum < 0) ? 0 : ((r_sum > 255) ? 255 : (uint8_t) r_sum);
			g = (g_sum < 0) ? 0 : ((g_sum > 255) ? 255 : (uint8_t) g_sum);
			b = (b_sum < 0) ? 0 : ((b_sum > 255) ? 255 : (uint8_t) b_sum);

			result[i][j] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}

	array_to_pixel_data(image, result);

	/* Free pixels as they have been replaced by result */
	for(i = 0; i < height; i++) {
		free(pixels[i]);
	}
	free(pixels);
	free_kernel(kernel, kernel_size);
}
