# Makefile for Transparent Game Engine - Farming Village Edition
# Pure C implementation

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -I. -Ilib
LDFLAGS = -lm

# Directories
SRC_DIR = src/core
LIB_DIR = lib
TEST_DIR = tests
BUILD_DIR = build

# Source files
LIB_SOURCES = $(LIB_DIR)/cJSON.c
CORE_SOURCES = $(SRC_DIR)/component.c $(SRC_DIR)/entity.c $(SRC_DIR)/game_state.c
ALL_SOURCES = $(LIB_SOURCES) $(CORE_SOURCES)

# Object files
LIB_OBJECTS = $(BUILD_DIR)/cJSON.o
CORE_OBJECTS = $(BUILD_DIR)/component.o $(BUILD_DIR)/entity.o $(BUILD_DIR)/game_state.o
ALL_OBJECTS = $(LIB_OBJECTS) $(CORE_OBJECTS)

# Test files
TEST_SOURCES = $(TEST_DIR)/test_phase1.c
TEST_EXECUTABLE = $(BUILD_DIR)/test_phase1

# Default target
all: $(BUILD_DIR) $(TEST_EXECUTABLE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile library objects
$(BUILD_DIR)/cJSON.o: $(LIB_DIR)/cJSON.c $(LIB_DIR)/cJSON.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile core objects
$(BUILD_DIR)/component.o: $(SRC_DIR)/component.c $(SRC_DIR)/component.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/entity.o: $(SRC_DIR)/entity.c $(SRC_DIR)/entity.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/game_state.o: $(SRC_DIR)/game_state.c $(SRC_DIR)/game_state.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test executable
$(TEST_EXECUTABLE): $(ALL_OBJECTS) $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(TEST_SOURCES) $(ALL_OBJECTS) -o $@ $(LDFLAGS)

# Run tests
test: $(TEST_EXECUTABLE)
	@echo "===================================================="
	@echo "Running Phase 1 Tests"
	@echo "===================================================="
	@./$(TEST_EXECUTABLE)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f saves/*.json

# Clean and rebuild
rebuild: clean all

# Install dependencies (nothing needed for pure C)
deps:
	@echo "All dependencies included (cJSON)"

# Help
help:
	@echo "Transparent Game Engine - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build everything (default)"
	@echo "  test      - Build and run tests"
	@echo "  clean     - Remove build artifacts"
	@echo "  rebuild   - Clean and rebuild"
	@echo "  deps      - Show dependencies"
	@echo "  help      - Show this help"

.PHONY: all test clean rebuild deps help
