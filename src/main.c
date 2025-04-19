#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "bitmap/read_image.h"
#include "bitmap/write_image.h"
#include "menu/menu.h"
#include "kernel/kernel.h"

int main(int argc, char* argv[]) {
	Image* image;
	int name_len;
	char* save_name;
	char* file_extension;
	char* file_ending = "-modified.bmp";

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
		return 1;
	}

	image = read_image(argv[1]);
	menu(image);

	file_extension = strrchr(argv[1], '.');
	name_len = file_extension - argv[1];
	save_name = malloc(name_len + strlen(file_ending) + 1);
	strncpy(save_name, argv[1], name_len);
	save_name[name_len] = '\0';
	strcat(save_name, file_ending);
	save_name[name_len + 14] = '\0';

	printf("Saving modified file to %s", save_name);
	write_image(save_name, image);
	free(save_name);

	return 0;
}
