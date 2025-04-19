#ifndef MODIFY_IMAGE_H
#define MODIFY_IMAGE_H

#include <stdint.h>
#include "read_image.h"
#include "../menu/menu.h"

void apply_kernel(Image* image, Menu kernel_type, int kernel_size);

#endif
