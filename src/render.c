#include "render.h"
#include <stdlib.h>

void set_pixel(u32 *buffer, int w, v2i pos, u32 color) {
  buffer[pos.y * w + pos.x] = color;
}

void draw_linei(u32 *buffer, int w, v2i p1, v2i p2, u32 color) {
  int dx = abs(p2.x - p1.x);
  int dy = abs(p2.y - p1.y);
  int sx = (p1.x < p2.x) ? 1 : -1;
  int sy = (p1.y < p2.y) ? 1 : -1;
  int d = dx - dy;

  for (;;) {
    set_pixel(buffer, w, p1, color);
    if (p1.x == p2.x && p1.y == p2.y) {
      break;
    }
    int d2 = 2 * d;
    if (d2 > -dy) {
      d -= dy;
      p1.x += sx;
    }
    if (d2 < dx) {
      d += dx;
      p1.y += sy;
    }
  }
}
