#include <stdint.h>
#include <stdlib.h>
#include "modify_image.h"
#include "write_image.h"
#include "read_image.h"
#include "../menu/menu.h"
#include "../kernel/kernel.h"
#include "../color/color.h"

void apply_kernel(Image* image, Menu kernel_type, int kernel_subtype, int kernel_size) {
	float** kernel;
	uint32_t** pixels;
	uint32_t** result;
	uint32_t height, width, i, j, ki, kj, ni, nj;
	uint8_t a, r, g, b;
	float a_sum, r_sum, g_sum, b_sum;
	int half_kernel = kernel_size / 2;

	switch(kernel_type) {
		case BLUR:
			if(kernel_subtype == GAUSSIAN) {
				kernel = create_guassian_blur_kernel(kernel_size);
			} else {
				kernel = create_box_blur_kernel(kernel_size);
			}
			break;
		case SHARPEN:
			if(kernel_subtype == GAUSSIAN) {
				kernel = create_guassian_sharpen_kernel(kernel_size);
			} else {
				kernel = create_sharpen_kernel(kernel_size);
			}
			break;
		case OUTLINE:
			kernel = create_outline_kernel(kernel_size);
			break;
		case EMBOSS:
			kernel = create_emboss_kernel(kernel_size, kernel_subtype);
			break;
		case SOBEL:
			kernel = create_sobel_kernel(kernel_size, kernel_subtype);
			break;
		default:
			fprintf(stderr, "Internal Error: Invalid kernel.");
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

void apply_hue_shift(Image* image, int amount, int step_count) {
	apply_hsl_modification(image, amount, step_count, MODIFY_HUE);
}

void apply_saturation(Image* image, int amount, int step_count) {
	apply_hsl_modification(image, amount, step_count, MODIFY_SATURATION);
}

void apply_brightness(Image* image, int amount, int step_count) {
	apply_hsl_modification(image, amount, step_count, MODIFY_BRIGHTNESS);
}

void apply_hsl_modification(Image* image, int amount, int step_count, HSLModification modification) {
	uint32_t height, width, i, j, argb;
	uint32_t** pixels = pixel_data_to_array(image);
	float step_size, modification_amount, h, s, l;
	uint8_t a;

	if (image->flags[INFO_HEADER]) {
		height = image->bm_info_header.height;
		width = image->bm_info_header.width;
	} else {
		height = image->bm_core_header.height;
		width = image->bm_core_header.width;
	}

	if (modification == MODIFY_HUE) {
		step_size = 360.0f / step_count;
		modification_amount = step_size * amount;
	} else {
		step_size = 1.0f / step_count;
		modification_amount = step_size * amount;
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			/* Get the pixels in HSL */
			rgb_to_hsl(pixels[i][j], &h, &s, &l, &a);

			switch(modification) {
				case MODIFY_HUE:
					/* Shift the hue and solve roll over shift */
					h += modification_amount;
					while(h >= 360.0f) {
						h -= 360.0f;
					}
					while(h < 0.0f) {
						h += 360.0f;
					}
					break;
				case MODIFY_SATURATION:
					/* Change saturation and clamp */
					s += modification_amount;
					s = (s < 0.0f) ? 0.0f : ((s > 1.0f) ? 1.0f : s);
					break;
				case MODIFY_BRIGHTNESS:
					/* Change brightness and clamp */
					l += modification_amount;
					l = (l < 0.0f) ? 0.0f : ((l > 1.0f) ? 1.0f : l);
					break;
			}

			/* Convert back to RGB and update pixel */
			hsl_to_rgb(h, s, l, a, &argb);
			pixels[i][j] = argb;
		}
	}

	array_to_pixel_data(image, pixels);
}
