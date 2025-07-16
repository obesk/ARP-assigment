CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -Wpedantic
LDFLAGS = -lm -lncurses -lcjson

BIN_DIR = bin
BUILD_DIR = build

# Executables
SPAWNER = $(BIN_DIR)/spawner
BLACKBOARD = $(BIN_DIR)/blackboard
DRONE = $(BIN_DIR)/drone
INPUT = $(BIN_DIR)/input
MAP = $(BIN_DIR)/map
TARGETS = $(BIN_DIR)/targets
OBSTACLES = $(BIN_DIR)/obstacles
WATCHDOG = $(BIN_DIR)/watchdog

LIB_SRC = src/pfds.c src/processes.c src/watchdog.c src/blackboard.c src/keys.c src/time_management.c src/vec2d.c

# Source files for each executable
SPAWNER_SRC = src/spawner.c $(LIB_SRC)
BLACKBOARD_SRC = src/blackboard_process.c $(LIB_SRC)
DRONE_SRC = src/drone_process.c $(LIB_SRC)
INPUT_SRC = src/input_process.c $(LIB_SRC)
MAP_SRC = src/map_process.c $(LIB_SRC)
TARGETS_SRC = src/targets_process.c $(LIB_SRC)
OBSTACLES_SRC = src/obstacles_process.c $(LIB_SRC)
WATCHDOG_SRC = src/watchdog_process.c $(LIB_SRC)

# Object files for each executable
SPAWNER_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SPAWNER_SRC))
BLACKBOARD_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(BLACKBOARD_SRC))
DRONE_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(DRONE_SRC))
INPUT_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(INPUT_SRC))
MAP_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(MAP_SRC))
TARGETS_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(TARGETS_SRC))
OBSTACLES_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(OBSTACLES_SRC))
WATCHDOG_OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(WATCHDOG_SRC))


# Default target
all: $(SPAWNER) $(BLACKBOARD) $(DRONE) $(INPUT) $(MAP) $(TARGETS) $(OBSTACLES) $(WATCHDOG)

# Build spawner executable
$(SPAWNER): $(SPAWNER_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SPAWNER_OBJ) $(LDFLAGS)

$(BLACKBOARD): $(BLACKBOARD_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(BLACKBOARD_OBJ) $(LDFLAGS)

$(WATCHDOG): $(WATCHDOG_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(WATCHDOG_OBJ) $(LDFLAGS)

$(DRONE): $(DRONE_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(DRONE_OBJ) $(LDFLAGS)

$(INPUT): $(INPUT_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(INPUT_OBJ) $(LDFLAGS)

$(MAP): $(MAP_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAP_OBJ) $(LDFLAGS)

$(TARGETS): $(TARGETS_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(TARGETS_OBJ) $(LDFLAGS)

$(OBSTACLES): $(OBSTACLES_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBSTACLES_OBJ) $(LDFLAGS)

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

kill:
	killall drone; killall blackboard; killall input; killall map; killall spawner; killall targets; killall obstacles; killall watchdog


