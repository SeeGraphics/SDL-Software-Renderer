#include "shapes.h"
#include "render.h"

void draw_triangle(u32 *buffer, int w, v2i p1, v2i p2, v2i p3, u32 color) {
  draw_linei(buffer, w, p1, p2, color);
  draw_linei(buffer, w, p2, p3, color);
  draw_linei(buffer, w, p3, p1, color);
}
