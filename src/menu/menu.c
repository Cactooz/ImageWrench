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
	{"Blur", BLUR},
	{"Sharpen", SHARPEN},
	{"Outline", OUTLINE},
	{"Emboss", EMBOSS},
	{"Sobel", SOBEL},
	{"Save & Exit", EXIT}
};
const int main_menu_options_count = 6;

MenuOption kernel_menu_options[] = {
	{"Kernel Size", OPTION_SIZE},
	{"Apply Filter", APPLY},
	{"Back", MAIN}
};
const int kernel_menu_options_count = 3;

MenuOption variable_kernel_menu_options[] = {
	{"Kernel Size", OPTION_SIZE},
	{"Kernel Type", OPTION_TYPE},
	{"Apply Filter", APPLY},
	{"Back", MAIN}
};
const int variable_kernel_menu_options_count = 4;

int kernel_size = 7;
int kernel_min_size = 3;
int kernel_max_size = 11;
int kernel_step_size = 2;

const char* blur_types[] = {"Box", "Guassian"};
const int blur_types_count = 2;
KernelType selected_blur_type = STANDARD;

const char* sharpen_types[] = {"Standard", "Gaussian"};
const int sharpen_types_count = 2;
KernelType selected_sharpen_type = STANDARD;

const int directions = 4;

const char* emboss_directions[] = {"Top Left", "Top Right", "Bottom Right", "Bottom Left"};
int selected_emboss_direction = 0;

const char* sobel_directions[] = {"Top", "Left", "Bottom", "Right"};
int selected_sobel_direction = 0;

int image_changed = 0;

int menu(Image* image) {
	int running = 1;
	Menu current_menu = MAIN;
	MenuOption choice;

	#ifndef _WIN32
		struct termios old_terminal_interface, new_terminal_interface;

		/* Save current terminal settings */
		tcgetattr(STDIN_FILENO, &old_terminal_interface);
		new_terminal_interface = old_terminal_interface;
		/* Disable waiting and echo */
		new_terminal_interface.c_lflag &= (~ICANON & ~ECHO);
		/* Apply new terminal settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal_interface);
	#endif

	while(running) {
		switch(current_menu) {
			case MAIN:
				choice = display_menu(MAIN, "ImageWrench", main_menu_options, main_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case BLUR:
				choice = display_menu(BLUR, "Blur Image", variable_kernel_menu_options, variable_kernel_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case SHARPEN:
				choice = display_menu(SHARPEN, "Sharpen Image", variable_kernel_menu_options, variable_kernel_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case OUTLINE:
				choice = display_menu(OUTLINE, "Outline Image", kernel_menu_options, kernel_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case EMBOSS:
				choice = display_menu(EMBOSS, "Emboss Image", variable_kernel_menu_options, variable_kernel_menu_options_count, image);
				current_menu = choice.menu;
				break;
			case SOBEL:
				choice = display_menu(SOBEL, "Sobel Image", variable_kernel_menu_options, variable_kernel_menu_options_count, image);
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
		/* Restore old terminal settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_interface);

		/* Restore terminal and show cursor */
		printf("\033[?25h");
		fflush(stdout);
	#endif

	return image_changed;
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
		/* Checks for ESC (arrow key escape sequence) */
		int c = getchar();
		if (c == 27) {
			c = getchar();
			if(c == '[') {
				c = getchar();
			}
		}

		return c;
	#endif
}

MenuOption display_menu(Menu current_menu, const char* title, MenuOption* options, int option_count, Image* image) {
	int key, i;
	int selected = 0;
	KernelType kernel_subtype = STANDARD;

	while(1) {
		clear_screen();
		printf("  %s\n\n", title);
		for(i = 0; i < option_count; i++) {
			if(i == selected) {
				printf("> ");
			} else {
				printf("  ");
			}

			printf("%s ", options[i].title);

			if(options[i].menu == OPTION_SIZE) {
				print_variable_menu(kernel_min_size, kernel_max_size, ((kernel_max_size - kernel_min_size) / kernel_step_size) + 1, kernel_size);
			} else if(options[i].menu == OPTION_TYPE) {
				switch(current_menu) {
					case(BLUR):
						print_type_selector(blur_types, blur_types_count, selected_blur_type);
						break;
					case(SHARPEN):
						print_type_selector(sharpen_types, sharpen_types_count, selected_sharpen_type);
						break;
					case(EMBOSS):
						print_type_selector(emboss_directions, directions, selected_emboss_direction);
						break;
					case(SOBEL):
						print_type_selector(sobel_directions, directions, selected_sobel_direction);
						break;
					default:
						break;
				}
			}
			printf("\n");
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
				if(options[selected].menu == OPTION_SIZE && kernel_size > kernel_min_size) {
					kernel_size -= kernel_step_size;
				} else if(options[selected].menu == OPTION_TYPE) {
					if(current_menu == BLUR && selected_blur_type > 0) {
						selected_blur_type--;
					} else if(current_menu == SHARPEN && selected_sharpen_type > 0) {
						selected_sharpen_type--;
					} else if(current_menu == EMBOSS && selected_emboss_direction > 0) {
						selected_emboss_direction--;
					} else if(current_menu == SOBEL && selected_sobel_direction > 0) {
						selected_sobel_direction--;
					}
				}
				break;
			/* Right arrow or d to increase option */
			case 67:
			case 77:
			case 'd':
				if(options[selected].menu == OPTION_SIZE && kernel_size < kernel_max_size) {
					kernel_size += kernel_step_size;
				} else if(options[selected].menu == OPTION_TYPE) {
					if(current_menu == BLUR && selected_blur_type < blur_types_count - 1) {
						selected_blur_type++;
					} else if(current_menu == SHARPEN && selected_sharpen_type < sharpen_types_count - 1) {
						selected_sharpen_type++;
					} else if(current_menu == EMBOSS && selected_emboss_direction < directions - 1) {
						selected_emboss_direction++;
					} else if(current_menu == SOBEL && selected_sobel_direction < directions - 1) {
						selected_sobel_direction++;
					}
				}
				break;
			/* LF and CR to exit/enter menu */
			case 10:
			case 13:
				if(options[selected].menu == APPLY) {
					printf(" ================================\n");
					printf(" | Applying kernel, please wait |\n");
					printf(" ================================\n");

					switch(current_menu) {
						case BLUR:
							kernel_subtype = selected_blur_type;
							break;
						case SHARPEN:
							kernel_subtype = selected_sharpen_type;
							break;
						case EMBOSS:
							kernel_subtype = selected_emboss_direction;
							break;
						case SOBEL:
							kernel_subtype = selected_sobel_direction;
							break;
						default:
							break;
					}
					
					apply_kernel(image, current_menu, kernel_subtype, kernel_size);
					image_changed++;
				} else if(options[selected].menu == OPTION_SIZE) {
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

void print_type_selector(const char** type_names, int type_count, int current_index) {
	if (current_index > 0) {
		printf("< ");
	} else {
		printf("  ");
	}

	printf("[%s]", type_names[current_index]);

	if (current_index < type_count - 1) {
		printf(" >");
	} else {
		 printf("  ");
	}
}
