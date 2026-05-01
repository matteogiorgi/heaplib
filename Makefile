CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic
CPPFLAGS := -Iinclude -Isrc


BUILD_DIR := build
LIB := $(BUILD_DIR)/libhpqlib.a
TARGET_TEST := $(BUILD_DIR)/priority_queue_test
TARGET_DIJKSTRA_BENCHMARK := $(BUILD_DIR)/dijkstra_benchmark
DEFAULT_DIMACS_GRAPH := graphs/dimacs/USA-road-d.NY.gr
SRC := src/priority_queue.c src/heaps/binary_heap.c src/heaps/fibonacci_heap.c src/heaps/kaplan_heap.c
OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)
SRC_TEST := tests/priority_queue_test.c
SRC_DIJKSTRA_BENCHMARK := tests/dijkstra_benchmark.c
PUBLIC_HEADERS := include/hpqlib/priority_queue.h
INTERNAL_HEADERS := src/priority_queue_internal.h src/heaps/binary_heap.h src/heaps/fibonacci_heap.h src/heaps/kaplan_heap.h
.PHONY: all clean test benchmark benchmark-smoke docs python-build python-test python-benchmark wheel release release-upload


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

$(TARGET_DIJKSTRA_BENCHMARK): $(SRC_DIJKSTRA_BENCHMARK) $(LIB) $(PUBLIC_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC_DIJKSTRA_BENCHMARK) $(LIB) -o $(TARGET_DIJKSTRA_BENCHMARK)

benchmark: $(TARGET_DIJKSTRA_BENCHMARK)
	./$(TARGET_DIJKSTRA_BENCHMARK) $(if $(GRAPH),$(GRAPH),$(DEFAULT_DIMACS_GRAPH)) $(if $(SOURCE),$(SOURCE),1)

benchmark-smoke: $(TARGET_DIJKSTRA_BENCHMARK)
	./$(TARGET_DIJKSTRA_BENCHMARK) $(DEFAULT_DIMACS_GRAPH) 1

docs:
	@if ! command -v doxygen >/dev/null 2>&1; then \
		echo "doxygen is required to build the C API documentation"; \
		exit 127; \
	fi
	doxygen Doxyfile

python-build:
	python3 setup.py build_ext --inplace

python-test: python-build
	python3 tests/priority_queue_test.py
	python3 tests/augmented_priority_queue.py

python-benchmark: python-build
	python3 tests/dijkstra_benchmark.py $(if $(GRAPH),$(GRAPH),$(DEFAULT_DIMACS_GRAPH)) $(if $(SOURCE),$(SOURCE),1)

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
