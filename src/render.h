#pragma once

#include "types.h"

v2i norm_to_screen(v2f norm, int w, int h);
v2f screen_to_norm(v2i screen, int w, int h);
void set_pixel(u32 *buffer, int w, v2i pos, u32 color);
void draw_linei(u32 *buffer, int w, v2i p1, v2i p2, u32 color);
