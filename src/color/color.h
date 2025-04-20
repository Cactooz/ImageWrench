#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

void rgb_to_hsl(uint32_t argb, float* h, float* s, float* l, uint8_t* a);
void hsl_to_rgb(float h, float s, float l, uint8_t a, uint32_t* argb);
float hue_to_rgb(float p, float q, float t);

#endif
