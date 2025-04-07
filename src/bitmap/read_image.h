#include <stdio.h>
#include "bitmap.h"

#ifndef READ_IMAGE_H
#define READ_IMAGE_H

enum ImageTypeFlags {
    BMv5=0,
    COLOR_TABLE=1,
    BITFIELDS=2,
    INFO_HEADER=3,
    RGB_TRIPLE = 4,
    GAP_TO_PIXEL = 5
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
    BMCoreHeader bm_core_header;
    BMInfoHeader bm_info_header;
    BMv5Header bm_v5_header;
    RGBQuad* color_table;
    RGBTriple* color_table_triple;
    uint32_t color_table_length;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint8_t* gap_to_pixel;
    uint32_t gap_to_pixel_len;
    uint8_t* pixel_data;
    uint32_t flags[6];
    uint8_t* rest_of_img;
} Image;

Image* read_image(char name[]);

int trailing_zeros_count(uint32_t number);

uint32_t** pixel_data_to_array(Image* image);

uint8_t* array_to_pixel_data(Image* image, uint32_t** array);

int find_closest_color(uint32_t color, RGBQuad* color_table, uint32_t color_table_length);

int color_distance(uint8_t red1, uint8_t red2, uint8_t green1, uint8_t green2, uint8_t blue1, uint8_t blue2);

#endif
