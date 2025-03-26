#include <stdio.h>
#include "menu.h"

#ifdef _WIN32
	#include <windows.h>
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif

const MenuOption main_menu_options[] = {
	{"Blur", BLUR, 0},
	{"Sharpen", SHARPEN, 0},
	{"Exit", EXIT, 0}
};
const short main_menu_options_count = 3;

const MenuOption kernel_menu_options[] = {
	{"Kernel Size: 3", OPTION, 3},
	{"Kernel Size: 5", OPTION, 5},
	{"Kernel Size: 7", OPTION, 7},
	{"Kernel Size: 9", OPTION, 9},
	{"Back", MAIN, 0}
};
const short kernel_menu_options_count = 5;

void menu(void) {
	int running = 1;
	Menu current_menu = MAIN;
	MenuOption choice;

	while(running) {
		switch(current_menu) {
			case MAIN:
				choice = display_menu("ImageWrench", main_menu_options, main_menu_options_count);
				current_menu = choice.menu;
				break;
			case BLUR:
				choice = display_menu("Blur Image", kernel_menu_options, kernel_menu_options_count);
				current_menu = choice.menu;
				break;
			case SHARPEN:
				choice = display_menu("Sharpen Image", kernel_menu_options, kernel_menu_options_count);
				current_menu = choice.menu;
				break;
			case OPTION:
				current_menu = MAIN;
				break;
			case EXIT:
				running = 0;
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
		short c = 0;

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

MenuOption display_menu(const char* title, const MenuOption* options, int option_count) {
	int key, i;
	int selected = 0;

	while(1) {
		clear_screen();
		printf("%s\n\n", title);
		for(i = 0; i < option_count; i++) {
			if(i == selected) {
				printf("> %s\n", options[i].title);
			} else {
				printf("  %s\n", options[i].title);
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
			/* LF and CR to exit/enter menu */
			case 10:
			case 13:
				return options[selected];
		}
	}
}
