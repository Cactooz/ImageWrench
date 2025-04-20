#ifndef MENU_H
#define MENU_H

#include "../bitmap/read_image.h"

typedef enum {
	MAIN,
	BLUR,
	SHARPEN,
	OUTLINE,
	EMBOSS,
	OPTION_SIZE,
	OPTION_TYPE,
	APPLY,
	EXIT
} Menu;

typedef enum { 
	STANDARD,
	GAUSSIAN 
} KernelType;

typedef struct {
	char* title;
	Menu menu;
} MenuOption;

int menu(Image* image);
void clear_screen(void);
int get_key(void);
MenuOption display_menu(Menu current_menu, const char* title, MenuOption* options, int option_count, Image* image);
void print_variable_menu(int min, int max, int steps, int current);
void print_type_selector(const char** type_names, int type_count, int current_index);

#endif
