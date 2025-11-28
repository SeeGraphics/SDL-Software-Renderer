CC := gcc
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS := $(shell sdl2-config --libs)
SDL_IMAGE_CFLAGS := $(shell pkg-config --cflags SDL2_image 2>/dev/null || pkg-config --cflags sdl2_image 2>/dev/null)
SDL_IMAGE_LIBS := $(shell pkg-config --libs SDL2_image 2>/dev/null || pkg-config --libs sdl2_image 2>/dev/null)

ifeq ($(SDL_IMAGE_LIBS),)
SDL_IMAGE_LIBS := -lSDL2_image
endif

CFLAGS := -Wall -std=c17 -g -fno-omit-frame-pointer -fsanitize=address,undefined $(SDL_CFLAGS) $(SDL_IMAGE_CFLAGS)
LIBS := $(SDL_LIBS) $(SDL_IMAGE_LIBS)

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
	$(CC) $(CFLAGS) $(SRCS) -o $(BIN) $(LIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
