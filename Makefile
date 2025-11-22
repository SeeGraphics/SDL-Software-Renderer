CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c17 -g -fno-omit-frame-pointer -fsanitize=address,undefined -lsdl2

SRC_DIR := src
BUILD_DIR := build
TARGET := game

SRCS := $(wildcard $(SRC_DIR)/*.c)
BIN := $(BUILD_DIR)/$(TARGET)

.PHONY: all run clean

all: $(BIN)

run: $(BIN)
	$(BIN)

clean:
	rm -rf $(BUILD_DIR)

$(BIN): $(SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
