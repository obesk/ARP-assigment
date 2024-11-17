CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -Wpedantic

BIN_DIR = bin
BUILD_DIR = build
TARGET = $(BIN_DIR)/simulation
SRC = src/spawner.c
OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) *.log
