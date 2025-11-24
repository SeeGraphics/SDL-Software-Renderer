#pragma once

#include "types.h"

#define WIREFRAME 0
#define FILLED 1

void draw_triangle(u32 *buffer, int w, v2i p1, v2i p2, v2i p3, u32 color,
                   u32 mode);
void draw_triangle_dots(u32 *buffer, int w, v2i p1, v2i p2, v2i p3, u32 color,
                        u32 mode);
void draw_cirlcei(u32 *buffer, int w, v2i pos, int r, u32 color, u32 mode);
