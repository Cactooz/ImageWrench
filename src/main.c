#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "bitmap/read_image.h"
#include "bitmap/write_image.h"
#include "menu/menu.h"

int main(int argc, char* argv[]) {
	Image* image;
	int name_len, modify_len, image_changed;
	FILE* test_file;
	char* save_name;
	char* file_extension;
	char* file_ending = "-modified.bmp";

	if(argc < 2) {
		fprintf(stderr, "Usage: %s image_path [output_path]\n", argv[0]);
		fprintf(stderr, "\timage_path: path to a .bmp image file\n");
		fprintf(stderr, "\toutput_path: path for the output .bmp image file (Optional)\n");
		return 1;
	}

	test_file = fopen(argv[1], "r");
	if(test_file == NULL) {
		fprintf(stderr, "Error: Invalid input path '%s'\n", argv[1]);
		fclose(test_file);
		return 1;
	}
	fclose(test_file);

	if(argc >= 3) {
		save_name = argv[2];
		test_file = fopen(save_name, "w");
		if(test_file == NULL) {
			fprintf(stderr, "Error: Invalid output path '%s'\n", save_name);
			fclose(test_file);
			return 1;
		}
		fclose(test_file);
	} else {
		file_extension = strrchr(argv[1], '.');
		name_len = file_extension - argv[1];
		modify_len = strlen(file_ending);
		save_name = malloc(name_len + modify_len + 1);
		strncpy(save_name, argv[1], name_len);
		save_name[name_len] = '\0';
		strcat(save_name, file_ending);
		save_name[name_len + modify_len] = '\0';
	}

	image = read_image(argv[1]);
	image_changed = menu(image);

	if(!image_changed) {
		printf("No changes were applied to the image.\n");
		return 0;
	}

	printf("Saving modified file to %s\n", save_name);
	write_image(save_name, image);

	/* Only free if no output path was given */
	if(argc < 3) {
		free(save_name);
	}

	return 0;
}
