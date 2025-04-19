#include <stdio.h>
#include <stdint.h>
#include "menu.h"
#include "../bitmap/read_image.h"
#include "../bitmap/modify_image.h"

#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif

MenuOption main_menu_options[] = {
	{"Blur", BLUR, -1, -1, -1, -1},
	{"Sharpen", SHARPEN, -1, -1, -1, -1},
	{"Save & Exit", EXIT, -1, -1, -1, -1}
};
const int main_menu_options_count = 3;

MenuOption kernel_menu_options[] = {
	{"Kernel Size", OPTION, 7, 3, 11, 2},
	{"Apply Filter", APPLY, -1, -1, -1, -1},
	{"Back", MAIN, -1, -1, -1, -1}
};
const int kernel_menu_options_count = 3;

void menu(Image* image) {
	int running = 1;
	Menu current_menu = MAIN;
	MenuOption choice;

	while(running) {
		switch(current_menu) {
			case MAIN:
				choice = display_menu(MAIN, "ImageWrench", main_menu_options, main_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case BLUR:
				choice = display_menu(BLUR, "Blur Image", kernel_menu_options, kernel_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case SHARPEN:
				choice = display_menu(SHARPEN, "Sharpen Image", kernel_menu_options, kernel_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case EXIT:
				running = 0;
				break;
			default:
				current_menu = MAIN;
				break;
		}
	}

	clear_screen();
	
	#ifndef _WIN32
		/* Restore terminal and show cursor */
		printf("\033[?25h");
		fflush(stdout);
	#endif
}

void clear_screen(void) {
	#ifdef _WIN32
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO console_buffer_info;
		COORD start_pos = {0, 0};
		DWORD console_character_count;
		DWORD written_chars;

		GetConsoleScreenBufferInfo(console, &console_buffer_info);
		console_character_count = console_buffer_info.dwSize.X * console_buffer_info.dwSize.Y;
		FillConsoleOutputCharacter(console, ' ', console_character_count, start_pos, &written_chars);
		SetConsoleCursorPosition(console, start_pos);
	#else
		/* Clear the screen, clear scrollback buffer, move curstor to top left */
		printf("\033[2J\033[3J\033[H");
		fflush(stdout);
	#endif
}

int get_key(void) {
	#ifdef _WIN32
		return _getch();
	#else
		struct termios old_terminal_interface, new_terminal_interface;
		int c = 0;

		/* Save current terminal settings */
		tcgetattr(STDIN_FILENO, &old_terminal_interface);
		new_terminal_interface = old_terminal_interface;
		/* Disable waiting and echo */
		new_terminal_interface.c_lflag &= (~ICANON & ~ECHO);
		/* Apply new terminal settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal_interface);

		/* Checks for ESC (arrow key escape sequence) */
		c = getchar();
		if (c == 27) {
			c = getchar();
			if(c == '[') {
				c = getchar();
			}
		}

		/* Restore old terminal settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_interface);
		return c;
	#endif
}

MenuOption display_menu(Menu current_menu, const char* title, MenuOption* options, int option_count, Image* image) {
	int key, i, kernel_size;
	int selected = 0;

	while(1) {
		clear_screen();
		printf("%s\n\n", title);
		for(i = 0; i < option_count; i++) {
			if(i == selected) {
				printf("> ");
			} else {
				printf("  ");
			}

			printf("%s ", options[i].title);

			if(options[i].option > 0) {
				print_variable_menu(options[i].min, options[i].max, ((options[i].max - options[i].min) / options[i].step_size) + 1, options[i].option);
			}
			printf("\n");

			if(options[i].menu == OPTION) {
				kernel_size = options[i].option;
			}
		}
		fflush(stdout);

		key = get_key();

		switch(key) {
			/* Up arrow or w to go up */
			case 65:
			case 72:
			case 'w':
				selected = (selected > 0) ? selected - 1 : option_count - 1;
				break;
			/* Down arrow or s to go down */
			case 66:
			case 80:
			case 's':
				selected = (selected < option_count - 1) ? selected + 1 : 0;
				break;
			/* Left arrow or a to decrese option */
			case 68:
			case 75:
			case 'a':
				if(options->option > options->min) {
					options->option -= options->step_size;
				}
				break;
			/* Right arrow or d to increase option */
			case 67:
			case 77:
			case 'd':
				if(options->option < options->max) {
					options->option += options->step_size;
				}
				break;
			/* LF and CR to exit/enter menu */
			case 10:
			case 13:
				if(options[selected].menu == APPLY) {
					printf(" ================================\n");
					printf(" | Applying kernel, please wait |\n");
					printf(" ================================\n");
					apply_kernel(image, current_menu, kernel_size);
				} else if(options[selected].menu == OPTION) {
					break;
				}
				return options[selected];
		}
	}
}

void print_variable_menu(int min, int max, int steps, int current) {
	int i;
	int step_size = ((max - min) / steps) + 1;

	if(current != min) {
		printf("< ");
	} else {
		printf("  ");
	}

	/* Print start of menu leading up to current value */
	printf("[");
	for(i = min; i < current; i += step_size) {
		printf("-");
	}

	/* Print the current value */
	if(current < 10) {
		printf(" 0");
	} else {
		printf(" ");
	}
	printf("%d ", current);

	/* Print the remaining part of the menu */
	for(i = current + 1; i < max; i += step_size) {
		printf("-");
	}
	printf("]");

	if(current != max) {
		printf(" >");
	}
}
