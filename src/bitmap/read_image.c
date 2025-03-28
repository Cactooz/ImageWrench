#include <stdlib.h>
#include <math.h>

#include "bitmap.h"
#include "read_image.h"

Image* read_image(char name[]) {
    Image *image = (Image*) malloc(sizeof(Image));
    FILE *file_pointer;
    BMPHeader bmp_header;
    DIBHeaderSize dib_header_size;
    BMInfoHeader bm_info_header;
    BMv5Header bm_v5_header;
    size_t items_read;
    uint8_t *pixel_data;
    uint8_t *rest_of_img;
    int bytes_left;
    uint32_t masks[3];
    uint32_t nr_of_colors;
    RGBQuad *color_table;
    
    /*Reset Flags*/
    image->flags[BMv5] = 0;
    image->flags[COLOR_TABLE] = 0;
    image->flags[BITFIELDS] = 0;

    /*Read file*/
    file_pointer = fopen(name, "rb");
    if (file_pointer == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", name);
        exit(2);
    }
    image->file = file_pointer;

    /*Read header*/
    items_read = fread(&bmp_header, sizeof(bmp_header), 1, file_pointer);
    if(items_read != 1) {
        fprintf(stderr, "Error: Unable to read BMP header from file %s\n", name);
        exit(3);
    }
    image->bmp_header = bmp_header;
    /*Read DIB Header*/
    items_read = fread(&dib_header_size, sizeof(dib_header_size), 1, file_pointer);
    if(items_read != 1) {
        fprintf(stderr, "Error: Unable to read DIB header size from file %s\n", name);
        exit(3);
    }
    image->dib_header_size = dib_header_size;

    
    items_read = fread(&bm_info_header, sizeof(bm_info_header), 1, file_pointer);
    if(items_read != 1) {
        fprintf(stderr, "Error: Unable to read DIB header from file %s\n", name);
        exit(3);
    }
    image->bm_info_header = bm_info_header;
    
    /*Check for unsupported bitmaps*/
    if(bm_info_header.bits_per_pixel != 1 &&
        bm_info_header.bits_per_pixel != 4 &&
        bm_info_header.bits_per_pixel != 8 &&
        bm_info_header.bits_per_pixel != 16 &&
        bm_info_header.bits_per_pixel != 24 && 
        bm_info_header.bits_per_pixel != 32) {
        fprintf(stderr, "Error: Unsupported bitmap type bits per pixel: %d\n", bm_info_header.bits_per_pixel);
        exit(4);
    }

    /*Read BMv5Header*/
    if(dib_header_size.type == 124) {
        image->flags[BMv5] = 1;
        items_read = fread(&bm_v5_header, sizeof(bm_v5_header), 1, file_pointer);
        if(items_read != 1) {
            fprintf(stderr, "Error: Unable to read DIB header from file %s\n", name);
            exit(3);
        }
        image->bm_v5_header = bm_v5_header;
    }

    /*Check for unsupported bitmaps*/
    if(dib_header_size.type != 40 && dib_header_size.type != 124) {
        fprintf(stderr, "Error: Unsupported bitmap header\n");
        exit(4);
    }

    /* Read color tables if bits per pixel <= 8 */
    if(bm_info_header.bits_per_pixel <= 8) {
        image->flags[COLOR_TABLE] = 1;
        nr_of_colors = bm_info_header.color_count;
        if(nr_of_colors == 0) 
            nr_of_colors = (uint32_t) pow(2, bm_info_header.bits_per_pixel);
        color_table = (RGBQuad *) malloc(sizeof(RGBQuad) * nr_of_colors);
        items_read = fread(color_table, sizeof(RGBQuad), nr_of_colors, file_pointer);
        if(items_read != nr_of_colors) {
            fprintf(stderr, "Error: Unable to read color table from file %s\n", name);
            exit(3);
        }
        image->color_table = color_table;
        image->color_table_length = nr_of_colors;
    }

    /* If compression is BITFIELDS (3), read color masks */
    if(bm_info_header.compression == 3) {
        image->flags[BITFIELDS] = 1;
        items_read = fread(masks, sizeof(uint32_t), 3, file_pointer);
        if(items_read != 3) {
            fprintf(stderr, "Error: Unable to read color masks from file %s\n", name);
            exit(3);
        }
        image->red_mask = masks[RED];
        image->green_mask = masks[GREEN];
        image->blue_mask = masks[BLUE];
    }
    
    if(bm_info_header.compression != 0 && bm_info_header.compression != 3) {
        fprintf(stderr, "Error: Unsupported bitmap compression in file %s\n", name);
        exit(4);
    }

    /*Read pixel data*/
    pixel_data = (uint8_t *) malloc(bm_info_header.image_size);
    items_read = fread(pixel_data, sizeof(uint8_t), bm_info_header.image_size, file_pointer);
    if(items_read != bm_info_header.image_size) {
        fprintf(stderr, "Error: Unable to read pixel data from file %s\n", name);
        exit(3);
    }
    image->pixel_data = pixel_data;
    
    
    /*Read rest of image*/
    bytes_left = bmp_header.size - bm_info_header.image_size - bmp_header.offset;
    rest_of_img = (uint8_t *) malloc(bytes_left);
    items_read = fread(rest_of_img, bytes_left, 1, file_pointer);
    if(items_read != 1 && bytes_left != 0) {
        fprintf(stderr, "Error: Unable to read data after pixel data from file %s\n", name);
        exit(3);
    }
    image->rest_of_img = rest_of_img;
    fclose(file_pointer);

    /*TODO: Make sure everything is free() correctly*/

    return image;
}

int trailing_zeros_count(uint32_t number) {
    int zeros = 0;
    if(number == 0)
        return 0;
    while(1) {
        if((number & 0x01) == 0) {
            zeros += 1;
            number = number >> 1;
        } else {
            break;
        }
    }
    return zeros;
}
uint32_t** pixel_data_to_array(Image* image) {
    uint32_t height = image->bm_info_header.height;
    uint32_t width = image->bm_info_header.width;
    uint32_t bits_per_pixel = image->bm_info_header.bits_per_pixel;
    uint32_t **array = (uint32_t **) malloc(height * sizeof(uint32_t*));
    uint32_t row_size;
    uint16_t pixel;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    int x, y, padding, pos;

    /* Each row contains a multiple of 4 bytes, otherwise padding is used */
    padding = (4 - ((width * bits_per_pixel / 8) % 4)) % 4;

    /*Row size including padding*/
    row_size = (width * bits_per_pixel) / 8 + padding;

    for(y = 0; y < height; y++)
        array[y] = (uint32_t *) malloc(sizeof(uint32_t) * width);

    for(x = 0; x < width; x++) {
        for(y = 0; y < height; y++) {
            if(bits_per_pixel == 1) {
                /* x * 0, we are on correct row */
                int byte = x / 8; /* 8 bits per byte */
                int offset = x % 8;
                uint8_t byte_value = image->pixel_data[y * row_size + byte];
                uint8_t bit_value = (byte_value >> (7 - offset)) & 0x01;
                array[y][x] = bit_value;
            } else if (bits_per_pixel == 4) {
                int byte = x / 2; /* 2 nibbles per byte */
                int offset = x % 2;
                uint8_t byte_value = image->pixel_data[y * row_size + byte];
                if(offset == 0) {
                    array[y][x] = (byte_value >> 4) & 0x0F;
                } else {
                    array[y][x] = byte_value & 0x0F;
                }
            } else if (bits_per_pixel == 8) {
                /* 8 bit is exactly one byte */
                array[y][x] = image->pixel_data[y * row_size + x];
            } else if(bits_per_pixel == 16) {
                /* Use masks to extract RGB */
                if(!image->flags[BITFIELDS]) {
                    image->red_mask = 0x7C00;
                    image->green_mask = 0x03E0;
                    image->blue_mask = 0x001F;
                }
                pos = y * row_size + x * 2;
                pixel = image->pixel_data[pos + 1] << 8 | image->pixel_data[pos];
                /* TODO: Remember when going back to pixel_data to use masks */
                red = (pixel & image->red_mask) >> trailing_zeros_count(image->red_mask);
                green = (pixel & image->green_mask) >> trailing_zeros_count(image->green_mask);
                blue = (pixel & image->blue_mask) >> trailing_zeros_count(image->blue_mask);
                array[y][x] = (0xFF << 24) | (red << 16) | (green << 8) | blue;

            } else if(bits_per_pixel == 24) {
                /* Extract 3 bytes as BGR and store it as ARGB, A = 0xFF */
                pos = y * row_size + x * 3;
                array[y][x] = (0xFF << 24) | (image->pixel_data[pos+2] << 16) | (image->pixel_data[pos+1] << 8) | image->pixel_data[pos];
            } else {
                /* 32 bit, grab 4 bytes (BGRA) and store in array as ARGB */
                pos = y * row_size + x * 4;
                array[y][x] = (image->pixel_data[pos+3] << 24) | (image->pixel_data[pos+2] << 16) | (image->pixel_data[pos+1] << 8) | image->pixel_data[pos];
            }
        }
    }

    /* If color table */
    if(image->flags[COLOR_TABLE]) {
        for(x = 0; x < width; x++) {
            for(y = 0; y < height; y++) {
                RGBQuad rgb = image->color_table[array[y][x]];
                printf("Red: %d Blue: %d Green: %d\n", image->color_table[0].red, image->color_table[0].blue, image->color_table[0].green);
                printf("%d\n", rgb.blue);
                array[y][x] = (0xFF << 24) | (rgb.red << 16) | (rgb.green << 8) | rgb.blue;
            }
        }
    }
    /*TODO: Currently, Y,X 0,0 down left. Stored row for row.*/
    return array;
}

uint8_t** array_to_pixel_data(Image* image, uint32_t** array) {
    return 0;
}


