#ifndef KERNEL_H
#define KERNEL_H

float** createKernel(int size);
void invertKernel(float** kernel, int size);

float** createBoxBlurKernel(int size);
float** createGaussianBlurKernel(int size);
float** createSharpenKernel(int size);
float** createGuassianSharpenKernel(int size);
float** createOutlineKernel(int size);

#endif
