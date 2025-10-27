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
CORE_SOURCES = $(SRC_DIR)/component.c $(SRC_DIR)/entity.c $(SRC_DIR)/game_state.c $(SRC_DIR)/event.c $(SRC_DIR)/decision.c $(SRC_DIR)/behavior.c $(SRC_DIR)/world.c $(SRC_DIR)/agriculture.c $(SRC_DIR)/economy.c
ALL_SOURCES = $(LIB_SOURCES) $(CORE_SOURCES)

# Object files
LIB_OBJECTS = $(BUILD_DIR)/cJSON.o
CORE_OBJECTS = $(BUILD_DIR)/component.o $(BUILD_DIR)/entity.o $(BUILD_DIR)/game_state.o $(BUILD_DIR)/event.o $(BUILD_DIR)/decision.o $(BUILD_DIR)/behavior.o $(BUILD_DIR)/world.o $(BUILD_DIR)/agriculture.o $(BUILD_DIR)/economy.o
ALL_OBJECTS = $(LIB_OBJECTS) $(CORE_OBJECTS)

# Test files
TEST_PHASE1 = $(BUILD_DIR)/test_phase1
TEST_PHASE2 = $(BUILD_DIR)/test_phase2
TEST_PHASE3 = $(BUILD_DIR)/test_phase3
TEST_PHASE4 = $(BUILD_DIR)/test_phase4
TEST_PHASE5 = $(BUILD_DIR)/test_phase5
TEST_PHASE6 = $(BUILD_DIR)/test_phase6
TEST_PHASE7 = $(BUILD_DIR)/test_phase7

# Default target
all: $(BUILD_DIR) $(TEST_PHASE1) $(TEST_PHASE2) $(TEST_PHASE3) $(TEST_PHASE4) $(TEST_PHASE5) $(TEST_PHASE6) $(TEST_PHASE7)

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

$(BUILD_DIR)/event.o: $(SRC_DIR)/event.c $(SRC_DIR)/event.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/decision.o: $(SRC_DIR)/decision.c $(SRC_DIR)/decision.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/behavior.o: $(SRC_DIR)/behavior.c $(SRC_DIR)/behavior.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/world.o: $(SRC_DIR)/world.c $(SRC_DIR)/world.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/agriculture.o: $(SRC_DIR)/agriculture.c $(SRC_DIR)/agriculture.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/economy.o: $(SRC_DIR)/economy.c $(SRC_DIR)/economy.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test executables
$(TEST_PHASE1): $(ALL_OBJECTS) $(TEST_DIR)/test_phase1.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_phase1.c $(ALL_OBJECTS) -o $@ $(LDFLAGS)

$(TEST_PHASE2): $(ALL_OBJECTS) $(TEST_DIR)/test_phase2.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_phase2.c $(ALL_OBJECTS) -o $@ $(LDFLAGS)

$(TEST_PHASE3): $(ALL_OBJECTS) $(TEST_DIR)/test_phase3.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_phase3.c $(ALL_OBJECTS) -o $@ $(LDFLAGS)

$(TEST_PHASE4): $(ALL_OBJECTS) $(TEST_DIR)/test_phase4.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_phase4.c $(ALL_OBJECTS) -o $@ $(LDFLAGS)

$(TEST_PHASE5): $(ALL_OBJECTS) $(TEST_DIR)/test_phase5.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_phase5.c $(ALL_OBJECTS) -o $@ $(LDFLAGS)

$(TEST_PHASE6): $(ALL_OBJECTS) $(TEST_DIR)/test_phase6.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_phase6.c $(ALL_OBJECTS) -o $@ $(LDFLAGS)

$(TEST_PHASE7): $(ALL_OBJECTS) $(TEST_DIR)/test_phase7.c
	$(CC) $(CFLAGS) $(TEST_DIR)/test_phase7.c $(ALL_OBJECTS) -o $@ $(LDFLAGS)

# Run all tests
test: test1 test2 test3 test4 test5 test6 test7

test1: $(TEST_PHASE1)
	@echo "===================================================="
	@echo "Running Phase 1 Tests"
	@echo "===================================================="
	@./$(TEST_PHASE1)

test2: $(TEST_PHASE2)
	@echo ""
	@echo "===================================================="
	@echo "Running Phase 2 Tests"
	@echo "===================================================="
	@./$(TEST_PHASE2)

test3: $(TEST_PHASE3)
	@echo ""
	@echo "===================================================="
	@echo "Running Phase 3 Tests"
	@echo "===================================================="
	@./$(TEST_PHASE3)

test4: $(TEST_PHASE4)
	@echo ""
	@echo "===================================================="
	@echo "Running Phase 4 Tests"
	@echo "===================================================="
	@./$(TEST_PHASE4)

test5: $(TEST_PHASE5)
	@echo ""
	@echo "===================================================="
	@echo "Running Phase 5 Tests"
	@echo "===================================================="
	@./$(TEST_PHASE5)

test6: $(TEST_PHASE6)
	@echo ""
	@echo "===================================================="
	@echo "Running Phase 6 Tests"
	@echo "===================================================="
	@./$(TEST_PHASE6)

test7: $(TEST_PHASE7)
	@echo ""
	@echo "===================================================="
	@echo "Running Phase 7 Tests"
	@echo "===================================================="
	@./$(TEST_PHASE7)

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
	@echo "  test      - Build and run all tests"
	@echo "  test1     - Run Phase 1 tests only"
	@echo "  test2     - Run Phase 2 tests only"
	@echo "  test3     - Run Phase 3 tests only"
	@echo "  test4     - Run Phase 4 tests only"
	@echo "  test5     - Run Phase 5 tests only"
	@echo "  test6     - Run Phase 6 tests only"
	@echo "  test7     - Run Phase 7 tests only"
	@echo "  clean     - Remove build artifacts"
	@echo "  rebuild   - Clean and rebuild"
	@echo "  deps      - Show dependencies"
	@echo "  help      - Show this help"

.PHONY: all test test1 test2 test3 test4 test5 test6 test7 clean rebuild deps help
