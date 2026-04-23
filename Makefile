CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic
CPPFLAGS := -Iinclude -Isrc


BUILD_DIR := build
LIB := $(BUILD_DIR)/libpqlib.a
TARGET_RUN := $(BUILD_DIR)/priority_queue_demo
TARGET_TEST := $(BUILD_DIR)/priority_queue_test
SRC := src/priority_queue.c src/heaps/binary_heap.c
OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)
SRC_DEMO := examples/priority_queue_demo.c
SRC_TEST := tests/priority_queue_test.c
PUBLIC_HEADERS := include/pqlib/priority_queue.h
INTERNAL_HEADERS := src/priority_queue_internal.h src/heaps/binary_heap.h
.PHONY: all clean run test


all: $(LIB)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c $(PUBLIC_HEADERS) $(INTERNAL_HEADERS) | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJ)
	ar rcs $(LIB) $(OBJ)

$(TARGET_RUN): $(SRC_DEMO) $(LIB) $(PUBLIC_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC_DEMO) $(LIB) -o $(TARGET_RUN)

$(TARGET_TEST): $(SRC_TEST) $(LIB) $(PUBLIC_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC_TEST) $(LIB) -o $(TARGET_TEST)

run: $(TARGET_RUN)
	./$(TARGET_RUN)

test: $(TARGET_TEST)
	./$(TARGET_TEST)

clean:
	rm -rf $(BUILD_DIR)
