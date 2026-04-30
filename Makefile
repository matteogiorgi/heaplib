CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic
CPPFLAGS := -Iinclude -Isrc


BUILD_DIR := build
LIB := $(BUILD_DIR)/libhpqlib.a
TARGET_TEST := $(BUILD_DIR)/priority_queue_test
SRC := src/priority_queue.c src/heaps/binary_heap.c src/heaps/fibonacci_heap.c src/heaps/kaplan_heap.c
OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)
SRC_TEST := tests/priority_queue_test.c
PUBLIC_HEADERS := include/hpqlib/priority_queue.h
INTERNAL_HEADERS := src/priority_queue_internal.h src/heaps/binary_heap.h src/heaps/fibonacci_heap.h src/heaps/kaplan_heap.h
.PHONY: all clean test python-build python-test wheel release release-upload


all: $(LIB)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c $(PUBLIC_HEADERS) $(INTERNAL_HEADERS) | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJ)
	rm -f $(LIB)
	ar rcs $(LIB) $(OBJ)

$(TARGET_TEST): $(SRC_TEST) $(LIB) $(PUBLIC_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC_TEST) $(LIB) -o $(TARGET_TEST)

test: $(TARGET_TEST)
	./$(TARGET_TEST)

python-build:
	python3 setup.py build_ext --inplace

python-test: python-build
	python3 tests/python_priority_queue_test.py
	python3 tests/learning_augmented_priority_queue_test.py

wheel:
	python3 -m pip wheel . --no-deps --no-build-isolation -w dist

release:
	@if [ -z "$(VERSION)" ]; then \
		echo "Usage: make release VERSION=0.1.0"; \
		exit 2; \
	fi
	./release.sh $(VERSION)

release-upload:
	@if [ -z "$(VERSION)" ]; then \
		echo "Usage: make release-upload VERSION=0.1.0 [REPO=OWNER/REPO]"; \
		exit 2; \
	fi
	@if [ -n "$(REPO)" ]; then \
		./release.sh $(VERSION) --upload --repo $(REPO); \
	else \
		./release.sh $(VERSION) --upload; \
	fi

clean:
	rm -rf $(BUILD_DIR)
	rm -rf dist
	rm -rf *.egg-info
	rm -rf src/*.egg-info
	rm -f *.so
	rm -f src/*.so
	rm -rf tests/__pycache__
	rm -rf tests/pq_experiments/__pycache__
