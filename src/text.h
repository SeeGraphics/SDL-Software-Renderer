#pragma once

#include "types.h"

void draw_char(u32 *buffer, int w, v2i pos, char c, u32 color);
void draw_text(u32 *buffer, int w, v2i pos, const char *text, u32 color);
