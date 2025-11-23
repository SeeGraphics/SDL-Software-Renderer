#pragma once

#include <stdint.h>

typedef struct {
  float x, y;
} v2f;

typedef struct {
  int x, y;
} v2i;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// shapes
typedef struct {
  v2i p1, p2, p3;
} Triangle;
