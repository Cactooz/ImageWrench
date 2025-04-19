#ifndef KERNEL_H
#define KERNEL_H

float** create_kernel(int size);
void free_kernel(float** kernel, int size);
void invert_kernel(float** kernel, int size);

float** create_box_blur_kernel(int size);
float** create_guassian_blur_kernel(int size);
float** create_sharpen_kernel(int size);
float** create_guassian_sharpen_kernel(int size);
float** create_outline_kernel(int size);

#endif
