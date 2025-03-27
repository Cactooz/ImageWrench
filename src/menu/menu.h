#ifndef MENU_H
#define MENU_H

typedef enum {
	MAIN,
	BLUR,
	SHARPEN,
	OPTION,
	EXIT
} Menu;

typedef struct {
	char* title;
	Menu menu;
	short option;
	short min;
	short max;
	short step_size;
} MenuOption;

void menu(void);
void clear_screen(void);
int get_key(void);
MenuOption display_menu(const char* title, MenuOption* options, short option_count);
void print_variable_menu(short min, short max, short steps, short current);

#endif
