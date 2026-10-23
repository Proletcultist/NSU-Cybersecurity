SRC_DIR = src
BUILD_DIR = build
OBJECTS_DIR = $(BUILD_DIR)/objects
INCLUDE_DIR = include
DEPS_DIR = $(BUILD_DIR)/deps
TEST_DIR = test
TEST_SAMPLES_DIR = $(TEST_DIR)/samples

CC = clang
CC_FLAGS = -Wall -Wextra -O0 -g
DEP_FLAGS = -MT $@ -MMD -MF $(DEPS_DIR)/$*.d

SOURCES = main.c gf256.c kuznechik.c
TEST_SAMPLES =	$(TEST_SAMPLES_DIR)/empty \
				$(TEST_SAMPLES_DIR)/less_than_block \
				$(TEST_SAMPLES_DIR)/exact_block \
				$(TEST_SAMPLES_DIR)/2mb
TEST_SCRIPT = $(TEST_DIR)/test.sh


$(BUILD_DIR)/main: $(patsubst %.c, $(OBJECTS_DIR)/%.o, $(SOURCES))
	mkdir -p $(BUILD_DIR)
	$(CC) -g -o $@ $^

$(OBJECTS_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	mkdir -p $(DEPS_DIR)
	mkdir -p $(OBJECTS_DIR)
	$(CC) -I $(INCLUDE_DIR) $(CC_FLAGS) $(DEP_FLAGS) -c -o $@ $<

$(TEST_SAMPLES_DIR)/empty:
	mkdir -p $(TEST_SAMPLES_DIR)
	touch $@
$(TEST_SAMPLES_DIR)/less_than_block:
	mkdir -p $(TEST_SAMPLES_DIR)
	head -c 64 /dev/urandom > $@
$(TEST_SAMPLES_DIR)/exact_block:
	mkdir -p $(TEST_SAMPLES_DIR)
	head -c 136 /dev/urandom > $@
$(TEST_SAMPLES_DIR)/2mb:
	mkdir -p $(TEST_SAMPLES_DIR)
	dd if=/dev/urandom of=$@ bs=1M count=2 status=none

test: $(BUILD_DIR)/main $(TEST_SCRIPT) $(TEST_SAMPLES)
	$(TEST_SCRIPT) $(BUILD_DIR)/main $(TEST_SAMPLES)

$(patsubst %.c, $(DEPS_DIR)/%.d, $(SOURCES)):

include $(patsubst %.c, $(DEPS_DIR)/%.d, $(SOURCES))

.PHONY: test
