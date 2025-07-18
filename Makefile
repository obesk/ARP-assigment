# useful dirs
BIN_DIR = bin
BUILD_DIR = build
CC_BUILD_DIR = $(BUILD_DIR)/c
CXX_BUILD_DIR = $(BUILD_DIR)/cpp
IDL_DIR = idl
IDL_BUILD_DIR = $(BUILD_DIR)/idl

CC = gcc
CXX = g++
CFLAGS = -Iinclude -Wall -Wextra -Wpedantic
CXXFLAGS = -std=c++11  -Iinclude -I$(IDL_BUILD_DIR) -I/usr/local/include/fastdds -I/usr/local/include/fastcdr
LDFLAGS = -lm -lncurses -lcjson
LDFLAGS_CXX = -lstdc++ -lm -lcjson -lfastcdr -lfastdds

# Executables
SPAWNER = $(BIN_DIR)/spawner
BLACKBOARD = $(BIN_DIR)/blackboard
DRONE = $(BIN_DIR)/drone
INPUT = $(BIN_DIR)/input
MAP = $(BIN_DIR)/map
TARGETS = $(BIN_DIR)/targets
OBSTACLES = $(BIN_DIR)/obstacles
WATCHDOG = $(BIN_DIR)/watchdog

#fastdds generated files
IDL_FILES = $(wildcard $(IDL_DIR)/*.idl)

#c files of the libraries
LIB_SRC = src/pfds.c src/processes.c src/watchdog.c src/blackboard.c src/keys.c \
	src/time_management.c src/vec2d.c src/config.c

# Source files for each executable
SPAWNER_SRC = src/spawner.c $(LIB_SRC)
BLACKBOARD_SRC = src/blackboard_process.c $(LIB_SRC)
DRONE_SRC = src/drone_process.c $(LIB_SRC)
INPUT_SRC = src/input_process.c $(LIB_SRC)
MAP_SRC = src/map_process.c $(LIB_SRC)
TARGETS_SRC = src/targets_process.c $(LIB_SRC)
OBSTACLES_SRC = src/obstacles_process.c $(LIB_SRC)
WATCHDOG_SRC = src/watchdog_process.c $(LIB_SRC)

PUBSUB_SRC = src/blackboard_publisher.cpp src/blackboard_publisher_cif.cpp \
	src/blackboard_subscriber.cpp src/blackboard_subscriber_cif.cpp

# This "predicts" the names of the generated files from fastdds
IDL_SRC = $(patsubst $(IDL_DIR)/%.idl,$(IDL_BUILD_DIR)/%TypeObjectSupport.cxx,$(IDL_FILES)) \
	$(patsubst $(IDL_DIR)/%.idl,$(IDL_BUILD_DIR)/%PubSubTypes.cxx,$(IDL_FILES))

FASTDDS_OBJ = $(patsubst $(IDL_BUILD_DIR)/%.cxx,$(IDL_BUILD_DIR)/%.o,$(IDL_SRC)) \
	$(patsubst src/%.cpp,$(CXX_BUILD_DIR)/%.o,$(PUBSUB_SRC))

# Object files for each executable
SPAWNER_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(SPAWNER_SRC))
BLACKBOARD_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(BLACKBOARD_SRC)) \
	$(FASTDDS_OBJ)
DRONE_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(DRONE_SRC))
INPUT_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(INPUT_SRC))
MAP_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(MAP_SRC))
TARGETS_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(TARGETS_SRC))
OBSTACLES_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(OBSTACLES_SRC))
WATCHDOG_OBJ = $(patsubst src/%.c,$(CC_BUILD_DIR)/%.o,$(WATCHDOG_SRC))


# Default target
all: $(SPAWNER) $(BLACKBOARD) $(DRONE) $(INPUT) $(MAP) $(TARGETS) $(OBSTACLES) $(WATCHDOG)

# Build spawner executable
$(SPAWNER): $(SPAWNER_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SPAWNER_OBJ) $(LDFLAGS)

# since the blackboard is using fastdds generated objects it needs to be linked
# with a C++ linker
$(BLACKBOARD): $(BLACKBOARD_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(BLACKBOARD_OBJ) $(LDFLAGS_CXX)

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
$(CC_BUILD_DIR)/%.o: src/%.c | $(CC_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files into object files
$(CXX_BUILD_DIR)/%.o: src/%.cpp | $(CXX_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

# Generate IDL files for fastdss
# Both rules will generate all the files, they are repeated so that the first 
# needed is used 
# TODO: remove ?
# .PRECIOUS: $(IDL_BUILD_DIR)/%TypeObjectSupport.cxx $(IDL_BUILD_DIR)/%PubSubTypes.cxx
$(IDL_BUILD_DIR)/%TypeObjectSupport.cxx: $(IDL_DIR)/%.idl | $(IDL_BUILD_DIR)
	fastddsgen $< -flat-output-dir -d $(IDL_BUILD_DIR)
$(IDL_BUILD_DIR)/%PubSubTypes.cxx: $(IDL_DIR)/%.idl | $(IDL_BUILD_DIR)
	fastddsgen $< -flat-output-dir -d $(IDL_BUILD_DIR)

$(IDL_BUILD_DIR)/%.o: $(IDL_BUILD_DIR)/%.cxx | $(IDL_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

# Create directories if they don't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(CC_BUILD_DIR):
	mkdir -p $(CC_BUILD_DIR)
$(CXX_BUILD_DIR):
	mkdir -p $(CXX_BUILD_DIR)
$(IDL_BUILD_DIR):
	mkdir -p $(IDL_BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) *.log

kill:
	killall drone; killall blackboard; killall input; killall map; killall spawner; killall targets; killall obstacles; killall watchdog


