#include <math.h>
#include <stdlib.h>

#include "write_image.h"
#include "read_image.h"

void write_image(char name[], Image* image) {
    FILE *file_pointer;
    uint32_t image_size;
    
    file_pointer = fopen(name, "wb");

    fwrite(&(image->bmp_header), sizeof(image->bmp_header), 1, file_pointer);

    fwrite(&(image->dib_header_size), sizeof(image->dib_header_size), 1, file_pointer);

    if(!image->flags[INFO_HEADER]) {
        fwrite(&(image->bm_core_header), sizeof(image->bm_core_header), 1, file_pointer);
        image_size = image->bmp_header.size - image->bmp_header.offset;
    }

    if(image->flags[INFO_HEADER]) {
        fwrite(&(image->bm_info_header), sizeof(image->bm_info_header), 1, file_pointer);
        image_size = image->bm_info_header.image_size;
    }

    if(image->flags[BMv5]) {
        fwrite(&(image->bm_v5_header), sizeof(image->bm_v5_header), 1, file_pointer);
    }

    if(image->flags[COLOR_TABLE]) {
        if(image->flags[RGB_TRIPLE]) {
            fwrite(image->color_table_triple, sizeof(RGBTriple), image->color_table_length, file_pointer);
        } else {
            fwrite(image->color_table, sizeof(RGBQuad), image->color_table_length, file_pointer);
        }
    }

    if (image->flags[BITFIELDS]) {
        fwrite(&(image->red_mask), (sizeof(image->red_mask)), 1, file_pointer);
        fwrite(&(image->green_mask), (sizeof(image->green_mask)), 1, file_pointer);
        fwrite(&(image->blue_mask), (sizeof(image->blue_mask)), 1, file_pointer);
    }

    if(image->flags[GAP_TO_PIXEL]) {
        fwrite(image->gap_to_pixel, image->gap_to_pixel_len, 1, file_pointer);
    }

    fwrite(image->pixel_data, image_size, 1, file_pointer);

    fwrite(image->rest_of_img, image->bmp_header.size - image_size - image->bmp_header.offset, 1, file_pointer);

    fclose(file_pointer);
    free_image(image);
}

void array_to_pixel_data(Image* image, uint32_t** array) {
    uint32_t height = image->bm_core_header.height;
    uint32_t width = image->bm_core_header.width;
    uint32_t bits_per_pixel = image->bm_core_header.bits_per_pixel;
    
    int padding;
    uint32_t row_size;
    uint8_t* pixel_data;
    int i = 0;
    int y;
    int x;
    int byte;
    int offset;
    int pos;
    int red_mask_zeros;
    int green_mask_zeros;
    int blue_mask_zeros;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
    uint8_t red_cap;
    uint8_t green_cap;
    uint8_t blue_cap;
    uint16_t pixel;

    if(image->flags[INFO_HEADER]) {
        height = image->bm_info_header.height;
        width = image->bm_info_header.width;
        bits_per_pixel = image->bm_info_header.bits_per_pixel;
    }

    padding = (4 - ((width * bits_per_pixel / 8) % 4)) % 4;
    row_size = (width * bits_per_pixel) / 8 + padding;
    pixel_data = malloc(height * row_size);

    for(i = 0; i < height * row_size; i++)
        pixel_data[i] = 0;
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            if(bits_per_pixel <= 8) {

                int index = find_closest_color(array[y][x], image->color_table, image->color_table_length);

                if(bits_per_pixel == 1) {
                    byte = x / 8;
                    offset = 7 - (x % 8);
                    /* Clear target bit */
                    pixel_data[y * row_size + byte] &= ~(1 << offset);
                    /* Set target bit */
                    pixel_data[y * row_size + byte] |= (index << offset);
                } else if(bits_per_pixel == 2) {
                    byte = x / 4;
                    offset = (3 - (x % 4)) * 2;
                    /* Clear target bit */
                    pixel_data[y * row_size + byte] &= ~(0x03 << offset);
                    /* Set target bit */
                    pixel_data[y * row_size + byte] |= (index << offset);
                } else if(bits_per_pixel == 4) {
                    byte = x / 2;
                    offset = (1 - (x % 2)) * 4;
                    /* Clear target bit */
                    pixel_data[y * row_size + byte] &= ~(0x0F << offset);
                    /* Set target bit */
                    pixel_data[y * row_size + byte] |= (index << offset);
                } else if(bits_per_pixel == 8) {
                    /* Color index is entire byte, simply set it */
                    pixel_data[y * row_size + x] = index;
                }
            } else if(bits_per_pixel == 16) {                
                if(!image->flags[BITFIELDS]) {
                    /*If no bitfields compression, use regular 5-5-5 RGB*/
                    image->red_mask = 0x7C00;
                    image->green_mask = 0x03E0;
                    image->blue_mask = 0x001F;
                }

                /* Get RGB values from ARGB array */
                red = (array[y][x] & 0x00FF0000) >> 16;
                green = (array[y][x] & 0x0000FF00) >> 8;
                blue = (array[y][x] & 0x000000FF);
                
                red_mask_zeros = trailing_zeros_count(image->red_mask);
                green_mask_zeros = trailing_zeros_count(image->green_mask);
                blue_mask_zeros = trailing_zeros_count(image->blue_mask);

                /* Cap values */
                red_cap = image->red_mask >> red_mask_zeros;
                green_cap = image->green_mask >> green_mask_zeros;
                blue_cap = image->blue_mask >> blue_mask_zeros;

                if(red > red_cap)
                    red = red_cap;
                if(green > green_cap)
                    green = green_cap;
                if(blue > blue_cap)
                    blue = blue_cap;

                pos = y * row_size + x * 2;
                pixel = (red << red_mask_zeros | green << green_mask_zeros | blue << blue_mask_zeros);

                pixel_data[pos] = (uint8_t)(pixel & 0xFF);
                pixel_data[pos + 1] = (uint8_t)(pixel >> 8);
            } else if(bits_per_pixel == 24) {
                /* Extract 3 bytes as BGR */
                pos = y * row_size + x * 3;
    
                red = (array[y][x] & 0x00FF0000) >> 16;
                green = (array[y][x] & 0x0000FF00) >> 8;
                blue = (array[y][x] & 0x000000FF);

                pixel_data[pos] = blue;
                pixel_data[pos+1] = green;
                pixel_data[pos+2] = red;
            } else if(bits_per_pixel == 32) {
                pos = y * row_size + x * 4;
                
                alpha = (array[y][x] & 0xFF000000) >> 24;
                red = (array[y][x] & 0x00FF0000) >> 16;
                green = (array[y][x] & 0x0000FF00) >> 8;
                blue = (array[y][x] & 0x000000FF);

                pixel_data[pos] = blue;
                pixel_data[pos+1] = green;
                pixel_data[pos+2] = red;
                pixel_data[pos+3] = alpha;
            } 
        }
    }
    /*Free original pixel data*/
    free(image->pixel_data);
    image->pixel_data = NULL;
    image->pixel_data = pixel_data;
    free_array(image, array);
}

int find_closest_color(uint32_t color, RGBQuad* color_table, uint32_t color_table_length) {
    uint8_t red = (color & 0x00FF0000) >> 16;
    uint8_t green = (color & 0x0000FF00) >> 8;
    uint8_t blue = color & 0x000000FF;
    RGBQuad rgb_quad;
    int distance;
    int closest_distance = color_distance(color_table[0].red, red, color_table[0].green, green, color_table[0].blue, blue);
    int i;
    int closest_index = 0;
    for (i = 1; i < color_table_length; i++) {
        rgb_quad = color_table[i];
        distance = color_distance(rgb_quad.red, red, rgb_quad.green, green, rgb_quad.blue, blue);
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_index = i;
        }
    }
    return closest_index;
}

int color_distance(uint8_t red1, uint8_t red2, uint8_t green1, uint8_t green2, uint8_t blue1, uint8_t blue2) {
    return sqrt(pow(red1 - red2, 2) + pow(green1 - green2, 2) + pow(blue1 - blue2, 2));
}

void free_image(Image* image) {
    if(image->flags[COLOR_TABLE])
        free(image->color_table);
    if(image->flags[RGB_TRIPLE])
        free(image->color_table_triple);
    if(image->flags[GAP_TO_PIXEL])
        free(image->gap_to_pixel);
    free(image->pixel_data);
    free(image->rest_of_img);
    free(image);
    image = NULL;
}

void free_array(Image* image, uint32_t** array) {
    int height;
    int y;
    if(image->flags[INFO_HEADER])
        height = image->bm_info_header.height;
    else
        height = image->bm_core_header.height;
    for(y = 0; y < height; y++)
        free(array[y]);
    free(array);
    array = NULL;
}
