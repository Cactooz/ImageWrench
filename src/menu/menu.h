#ifndef MENU_H
#define MENU_H

#include "../bitmap/read_image.h"

typedef enum {
	MAIN,
	BLUR,
	SHARPEN,
	OUTLINE,
	OPTION,
	APPLY,
	EXIT
} Menu;

typedef struct {
	char* title;
	Menu menu;
} MenuOption;

void menu(Image* image);
void clear_screen(void);
int get_key(void);
MenuOption display_menu(Menu current_menu, const char* title, MenuOption* options, int option_count, Image* image);
void print_variable_menu(int min, int max, int steps, int current);

#endif
