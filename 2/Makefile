CC = clang
CC_FLAGS = -Wall -Wextra

SRC_DIR = src
BUILD_DIR = build
OBJECTS_DIR = $(BUILD_DIR)/objects
INCLUDE_DIR = include

SOURCES = main.c sponge.c

$(BUILD_DIR)/main: $(patsubst %.c, $(OBJECTS_DIR)/%.o, $(SOURCES))
	$(CC) -o $@ $^

$(OBJECTS_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJECTS_DIR)
	$(CC) -I $(INCLUDE_DIR) $(CC_FLAGS) -c -o $@ $^
