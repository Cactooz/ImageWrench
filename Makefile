FLAGS = -Wall -Werror -ansi -pedantic

SRC_DIR = src
BUILD_DIR = build

TARGET = $(BUILD_DIR)/ImageWrench.exe

all:
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	gcc $(FLAGS) $(SRC_DIR)/main.c -o $(TARGET)
