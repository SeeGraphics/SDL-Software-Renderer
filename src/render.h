#pragma once

#include "types.h"
#include <stdlib.h>

void set_pixel(u32 *buffer, int w, v2i pos, u32 color);
void draw_linei(u32 *buffer, int w, v2i p1, v2i p2, u32 color);
