#ifndef MODIFY_IMAGE_H
#define MODIFY_IMAGE_H

#include <stdint.h>
#include "read_image.h"
#include "../menu/menu.h"

typedef enum { 
	MODIFY_HUE,
	MODIFY_SATURATION,
	MODIFY_BRIGHTNESS 
} HSLModification;

void apply_kernel(Image* image, Menu kernel_type, int kernel_subtype, int kernel_size);
void apply_hue_shift(Image* image, int amount, int step_count);
void apply_saturation(Image* image, int amount, int step_count);
void apply_brightness(Image* image, int amount, int step_count);
void apply_hsl_modification(Image* image, int amount, int step_count, HSLModification modification);

#endif
