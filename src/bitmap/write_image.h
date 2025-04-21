#include "read_image.h"

#ifndef WRITE_IMAGE_H
#define WRITE_IMAGE_H

void write_image(char name[], Image* image);

void items_written_check(size_t items_written, size_t expected_items, char* msg, char* name, FILE* file_pointer, Image* image);

void array_to_pixel_data(Image* image, uint32_t** array);

int find_closest_color(uint32_t color, RGBQuad* color_table, uint32_t color_table_length);

int color_distance(uint8_t red1, uint8_t red2, uint8_t green1, uint8_t green2, uint8_t blue1, uint8_t blue2);

void free_image(Image* image);

void free_array(Image* image, uint32_t** array);

#endif
