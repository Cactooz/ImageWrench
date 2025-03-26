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
} MenuOption;

void menu(void);
void clear_screen(void);
int get_key(void);
MenuOption display_menu(const char* title, const MenuOption* options, int option_count);

#endif
