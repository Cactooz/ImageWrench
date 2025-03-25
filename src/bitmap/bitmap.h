#include <stdint.h>

typedef struct {
	uint16_t type;
	uint32_t size;
	uint32_t reserved;
	uint32_t offset;
} BMPHeader;

typedef struct {
	uint32_t type;
} DIBHeaderSize;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint16_t color_planes;
	uint16_t bits_per_pixel;
	uint32_t compression;
	uint32_t size_image;
	uint32_t horizontal_resolution;
	uint32_t vertical_resolution;
	uint32_t color_count;
	uint32_t important_color_count;
} BMInfoHeader;

typedef struct {
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t alpha_mask;
	uint32_t cs_type;
	uint32_t cie_red_x;
	uint32_t cie_red_y;
	uint32_t cie_red_z;
	uint32_t cie_green_x;
	uint32_t cie_green_y;
	uint32_t cie_green_z;
	uint32_t cie_blue_x;
	uint32_t cie_blue_y;
	uint32_t cie_blue_z;
	uint32_t gamma_red;
	uint32_t gamma_green;
	uint32_t gamma_blue;
	uint32_t intent;
	uint32_t profile_data;
	uint32_t profile_size;
	uint32_t reserved;
} BMv5Header;
