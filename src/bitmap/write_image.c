#include "read_image.h"

void write_image(char name[], Image* image, uint8_t* new_pixel_data) {
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

    fwrite(new_pixel_data, image_size, 1, file_pointer);

    fwrite(image->rest_of_img, image->bmp_header.size - image_size - image->bmp_header.offset, 1, file_pointer);

    fclose(file_pointer);
}
