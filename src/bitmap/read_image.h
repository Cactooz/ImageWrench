#include <stdio.h>
#include "bitmap.h"

#ifndef READ_IMAGE_H
#define READ_IMAGE_H

enum ImageTypeFlags {
    BMv5=0,
    COLOR_TABLE=1,
    BITFIELDS=2
};

enum ColorMasks {
    RED=0,
    GREEN=1,
    BLUE=2
};

typedef struct {
    FILE* file;
    BMPHeader bmp_header;
    DIBHeaderSize dib_header_size;
    BMInfoHeader bm_info_header;
    BMv5Header bm_v5_header;
    RGBQuad* color_table;
    uint32_t color_table_length;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint8_t* pixel_data;
    uint32_t flags[3];
    uint8_t* rest_of_img;
} Image;

Image* read_image(char name[]);

int trailing_zeros_count(uint32_t number);

uint32_t** pixel_data_to_array(Image* image);

uint8_t** array_to_pixel_data(Image* image, uint32_t** array);

#endif
