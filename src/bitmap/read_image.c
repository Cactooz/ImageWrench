#include <stdlib.h>
#include <math.h>

#include "bitmap.h"
#include "read_image.h"

Image* read_image(char name[]) {
    Image *image = (Image*) malloc(sizeof(Image));
    FILE *file_pointer;
    BMPHeader bmp_header;
    DIBHeaderSize dib_header_size;
    BMCoreHeader bm_core_header;
    BMInfoHeader bm_info_header;
    BMv5Header bm_v5_header;
    size_t items_read;
    uint8_t *pixel_data;
    uint8_t *rest_of_img;
    int bytes_left;
    uint32_t masks[3];
    uint32_t nr_of_colors;
    RGBQuad *color_table;
    RGBTriple *color_table_temp;
    int i;
    uint16_t bits_per_pixel;
    uint32_t image_size;
    int bytes_read = 0;
    uint8_t* gap_to_pixel;
    
    /*Reset Flags*/
    image->flags[BMv5] = 0;
    image->flags[COLOR_TABLE] = 0;
    image->flags[BITFIELDS] = 0;
    image->flags[INFO_HEADER] = 0;
    image->flags[RGB_TRIPLE] = 0;
    image->flags[GAP_TO_PIXEL] = 0;

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
    bytes_read += sizeof(bmp_header);
    image->bmp_header = bmp_header;

    /*Read DIB Header Size*/
    items_read = fread(&dib_header_size, sizeof(dib_header_size), 1, file_pointer);
    if(items_read != 1) {
        fprintf(stderr, "Error: Unable to read DIB header size from file %s\n", name);
        exit(3);
    }
    bytes_read += sizeof(dib_header_size);
    image->dib_header_size = dib_header_size;

    /*Check for unsupported bitmaps*/
    if(dib_header_size.type != 12 && dib_header_size.type != 40 && dib_header_size.type != 124) {
        fprintf(stderr, "Error: Unsupported bitmap header\n");
        exit(4);
    }

    /*Read Bitmap Core Header*/
    if(dib_header_size.type == 12) {
        items_read = fread(&bm_core_header, sizeof(bm_core_header), 1, file_pointer);
        if(items_read != 1) {
            fprintf(stderr, "Error: Unable to read BM Core Header from file %s\n", name);
            exit(3);
        }
        image->bm_core_header = bm_core_header;
        bits_per_pixel = bm_core_header.bits_per_pixel;
        image_size = bmp_header.size - bmp_header.offset;
        bytes_read += sizeof(bm_core_header);
    }

    /*Read Bitmap Info Header*/
    if(dib_header_size.type >= 40) {
        image->flags[INFO_HEADER] = 1;
        items_read = fread(&bm_info_header, sizeof(bm_info_header), 1, file_pointer);
        if(items_read != 1) {
            fprintf(stderr, "Error: Unable to read DIB header from file %s\n", name);
            exit(3);
        }
        bytes_read += sizeof(bm_info_header);
        image->bm_info_header = bm_info_header;
        bits_per_pixel = bm_info_header.bits_per_pixel;
        image_size = bm_info_header.image_size;
    }
    
    /*Check for unsupported bitmaps*/
    if(bits_per_pixel != 1 &&
        bits_per_pixel != 4 &&
        bits_per_pixel != 8 &&
        /*bits_per_pixel != 16 && TODO: Fix 16 bpp support*/
        bits_per_pixel != 24 && 
        bits_per_pixel != 32) {
        fprintf(stderr, "Error: Unsupported bitmap type bits per pixel: %d\n", bits_per_pixel);
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
        bytes_read += sizeof(bm_v5_header);
        image->bm_v5_header = bm_v5_header;
    }

    /* Read color tables if bits per pixel <= 8 */
    if(bits_per_pixel <= 8) {
        image->flags[COLOR_TABLE] = 1;
        nr_of_colors = 0;
        if(image->flags[INFO_HEADER]) {
            nr_of_colors = bm_info_header.color_count;
        }
        if(nr_of_colors == 0) 
            nr_of_colors = (uint32_t) pow(2, bits_per_pixel);

        if(image->flags[INFO_HEADER]) {
            color_table = (RGBQuad *) malloc(sizeof(RGBQuad) * nr_of_colors);
            items_read = fread(color_table, sizeof(RGBQuad), nr_of_colors, file_pointer);
            bytes_read += sizeof(RGBQuad) * nr_of_colors;
        } else {
            /*If BM Core Header, then RGB Triples instead of quads*/
            color_table_temp = (RGBTriple *) malloc(sizeof(RGBTriple) * nr_of_colors);
            items_read = fread(color_table_temp, sizeof(RGBTriple), nr_of_colors, file_pointer);
            color_table = (RGBQuad *) malloc(sizeof(RGBQuad) * nr_of_colors);
            bytes_read += sizeof(RGBTriple) * nr_of_colors;

            /*Translate RGB Triples to RGB Quads*/
            for(i = 0; i < nr_of_colors; i++) {
                RGBQuad rgb;
                rgb.red = color_table_temp[i].red;
                rgb.green = color_table_temp[i].green;
                rgb.blue = color_table_temp[i].blue;
                rgb.reserved = 0xFF;
                color_table[i] = rgb;
            }
            image->color_table_triple = color_table_temp;
            image->flags[RGB_TRIPLE] = 1;
        }
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
        bytes_read += sizeof(uint32_t) * 3;
        image->red_mask = masks[RED];
        image->green_mask = masks[GREEN];
        image->blue_mask = masks[BLUE];
    }
    
    /*Check for unsupported bitmaps, TODO: Add RLE compression*/
    if(image->flags[INFO_HEADER] && bm_info_header.compression != 0 && bm_info_header.compression != 3) {
        fprintf(stderr, "Error: Unsupported bitmap compression in file %s\n", name);
        exit(4);
    }

    /*Read pixel data*/
    /*If not at offset yet, then there is a gap to pixel data*/
    if(bytes_read != bmp_header.offset) {
        image->flags[GAP_TO_PIXEL] = 1;
        image->gap_to_pixel_len = bmp_header.offset - bytes_read;
        gap_to_pixel = (uint8_t *) malloc(image->gap_to_pixel_len * sizeof(uint8_t));
        items_read = fread(gap_to_pixel, sizeof(uint8_t), image->gap_to_pixel_len, file_pointer);
        if(items_read != sizeof(uint8_t) * image->gap_to_pixel_len) {
            fprintf(stderr, "Error: Unable to read pixel data from file %s\n", name);
            exit(3);
        }
        image->gap_to_pixel = gap_to_pixel;
    }

    pixel_data = (uint8_t *) malloc(image_size * sizeof(uint8_t));
    items_read = fread(pixel_data, sizeof(uint8_t), image_size, file_pointer);
    if(items_read != image_size) {
        fprintf(stderr, "Error: Unable to read pixel data from file %s\n", name);
        exit(3);
    }
    image->pixel_data = pixel_data;
    
    
    /*Read rest of image*/
    bytes_left = bmp_header.size - image_size - bmp_header.offset;
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

uint32_t** pixel_data_to_array(Image* image) {
    uint32_t height = image->bm_core_header.height;
    uint32_t width = image->bm_core_header.width;
    uint32_t bits_per_pixel = image->bm_core_header.bits_per_pixel;
    uint32_t **array;
    uint32_t row_size;
    uint16_t pixel;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    int x, y, padding, pos;

    if(image->flags[INFO_HEADER]) {
        height = image->bm_info_header.height;
        width = image->bm_info_header.width;
        bits_per_pixel = image->bm_info_header.bits_per_pixel;
    }

    array = (uint32_t **) malloc(height * sizeof(uint32_t*));

    /* Each row contains a multiple of 4 bytes, otherwise padding is used */
    padding = (4 - ((width * bits_per_pixel / 8) % 4)) % 4;

    /*Row size including padding*/
    row_size = (width * bits_per_pixel) / 8 + padding;

    for(y = 0; y < height; y++)
        array[y] = (uint32_t *) malloc(sizeof(uint32_t) * width);

    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            if(bits_per_pixel == 1) {
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
                    /*If no bitfields compression, use regular 5-5-5 RGB*/
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
        for(y = 0; y < height; y++) {
            for(x = 0; x < width; x++) {
                RGBQuad rgb = image->color_table[array[y][x]];
                array[y][x] = (0xFF << 24) | (rgb.red << 16) | (rgb.green << 8) | rgb.blue;
            }
        }
    }
    
    return array;
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

uint8_t* array_to_pixel_data(Image* image, uint32_t** array) {
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
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

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
                /* TOOD: Use correct masks to set it properly */

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
    return pixel_data;
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
