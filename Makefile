FLAGS = -Wall -Werror -ansi -pedantic

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

ifeq ($(OS), Windows_NT)
	TARGET = $(BIN_DIR)/ImageWrench.exe
	MKDIR = if not exist "$1" mkdir "$1"
	RMDIR = if exist "$1" rmdir /s /q "$1"
else
	TARGET = $(BIN_DIR)/ImageWrench
	MKDIR = mkdir -p "$1"
	RMDIR = rm -rf "$1"
endif

SRC_FILES = $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(call MKDIR,$(@D))
	gcc $(FLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	@$(call MKDIR,$(BIN_DIR))
	gcc $(FLAGS) $^ -o $(TARGET)

clean:
	@$(call RMDIR,$(BIN_DIR))
	@$(call RMDIR,$(BUILD_DIR))

debug:
	@echo Source files found: $(SRC_FILES)
	@echo Object files to build: $(OBJ_FILES)

.PHONY: clean debug
