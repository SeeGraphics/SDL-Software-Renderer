#pragma once

#include <stdint.h>

typedef struct {
  float x, y;
} v2f;

typedef struct {
  int x, y;
} v2i;

typedef struct {
  float x, y, z;
} v3f;

typedef struct {
  float x, y, z, w;
} v4f;

typedef struct {
  float m[4][4];
} mat4;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// shapes
typedef struct {
  v2i p1, p2, p3;
} Triangle;

typedef struct {
  int w;
  int h;
  u32 *pixels;
} Texture;

typedef struct {
  v2i pos;
  v2f uv;
} VertexUV;

typedef struct {
  v2i pos;
  v2f uv;
  float inv_w;
  float depth;
} VertexPC;

typedef struct {
  v3f pos;
  v2f uv;
} Vertex3D;
