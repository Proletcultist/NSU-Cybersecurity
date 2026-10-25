SRC_DIR = src
BUILD_DIR = build
OBJECTS_DIR = $(BUILD_DIR)/objects
INCLUDE_DIR = include
DEPS_DIR = $(BUILD_DIR)/deps
LIBS = -lsodium
TEST_DIR = test
TEST_SAMPLES_DIR = $(TEST_DIR)/samples

CC = clang
CC_FLAGS = -Wall -Wextra -O0
DEP_FLAGS = -MT $@ -MMD -MF $(DEPS_DIR)/$*.d

SOURCES = main.c gf256.c kuznechik.c

$(BUILD_DIR)/main: $(patsubst %.c, $(OBJECTS_DIR)/%.o, $(SOURCES))
	mkdir -p $(BUILD_DIR)
	$(CC) $(LIBS) -o $@ $^

$(OBJECTS_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	mkdir -p $(DEPS_DIR)
	mkdir -p $(OBJECTS_DIR)
	$(CC) -I $(INCLUDE_DIR) $(CC_FLAGS) $(DEP_FLAGS) -c -o $@ $<

$(patsubst %.c, $(DEPS_DIR)/%.d, $(SOURCES)):

include $(patsubst %.c, $(DEPS_DIR)/%.d, $(SOURCES))

.PHONY: test
