#include <stdlib.h>

#include "bitmap.h"
#include "read_image.h"

Image* read_image(char name[]) {
    Image *image = (Image*) malloc(sizeof(Image));
    FILE *file_pointer;
    BMPHeader bmp_header;
    DIBHeaderSize dib_header_size;
    BMInfoHeader bm_info_header;
    BMv5Header bm_v5_header;
    size_t bytes_read;

    /*Read file*/
    file_pointer = fopen(name, "rb");
    if (file_pointer == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", name);
        exit(2);
    }
    image->file = file_pointer;

    /*Read header*/
    bytes_read = fread(&bmp_header, sizeof(bmp_header), 1, file_pointer);
    if(bytes_read != 1) {
        fprintf(stderr, "Error: Unable to read BMP header from file %s\n", name);
        exit(3);
    }
    /*Read DIB Header*/
    bytes_read = fread(&dib_header_size, sizeof(dib_header_size), 1, file_pointer);
    if(bytes_read != 1) {
        fprintf(stderr, "Error: Unable to read DIB header size from file %s\n", name);
        exit(3);
    }
    image->dib_header_size = dib_header_size;

    
    bytes_read = fread(&bm_info_header, sizeof(bm_info_header), 1, file_pointer);
    if(bytes_read != 1) {
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
        bytes_read = fread(&bm_v5_header, sizeof(bm_v5_header), 1, file_pointer);
        if(bytes_read != 1) {
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

    /* TODO: Read color tables if bits per pixel <= 8 ??? */

    /* TODO: compression = BITFIELDS, more info exists... */
    return image;
}
