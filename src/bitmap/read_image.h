#include <stdio.h>
#include "bitmap.h"

#ifndef READ_IMAGE_H
#define READ_IMAGE_H
typedef struct {
    FILE* file;
    BMPHeader bmp_header;
    DIBHeaderSize dib_header_size;
    BMInfoHeader bm_info_header;
    BMv5Header bm_v5_header;
} Image;

Image* read_image(char name[]);

#endif
