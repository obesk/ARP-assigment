CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -Wpedantic
LDFLAGS = -lm

BIN_DIR = bin
BUILD_DIR = build

# Executables
SPAWNER = $(BIN_DIR)/spawner
BLACKBOARD = $(BIN_DIR)/blackboard
DRONE = $(BIN_DIR)/drone

# Source files for each executable
SPAWNER_SRC = src/spawner.c
BLACKBOARD_SRC = src/blackboard_process.c
DRONE_SRC = src/drone_process.c

# Object files for each executable
SPAWNER_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SPAWNER_SRC))
BLACKBOARD_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(BLACKBOARD_SRC))
DRONE_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(DRONE_SRC))

# Default target
all: $(SPAWNER) $(BLACKBOARD) $(DRONE)

# Build spawner executable
$(SPAWNER): $(SPAWNER_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SPAWNER_OBJ) $(LDFLAGS)

# Build blackboard executable
$(BLACKBOARD): $(BLACKBOARD_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(BLACKBOARD_OBJ) $(LDFLAGS)

$(DRONE): $(DRONE_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(DRONE_OBJ) $(LDFLAGS)

# Compile source files into object files
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) *.log

