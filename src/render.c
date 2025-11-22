#include "render.h"
#include <SDL2/SDL.h>

void set_pixel(u32 *buffer, int w, v2i pos, u32 color) {
  buffer[pos.y * w + pos.x] = color;
}
