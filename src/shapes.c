#include "shapes.h"
#include "colors.h"
#include "render.h"

void draw_triangle(u32 *buffer, int w, v2i p1, v2i p2, v2i p3, u32 color,
                   u32 mode) {
  if (mode == WIREFRAME) {
    draw_linei(buffer, w, p1, p2, color);
    draw_linei(buffer, w, p2, p3, color);
    draw_linei(buffer, w, p3, p1, color);
  }
  if (mode == FILLED) {
    draw_linei(buffer, w, p1, p2, color);
    draw_linei(buffer, w, p2, p3, color);
    draw_linei(buffer, w, p3, p1, color);
  }
}

void draw_triangle_dots(u32 *buffer, int w, v2i p1, v2i p2, v2i p3, u32 color,
                        u32 mode) {
  if (mode == WIREFRAME) {
    draw_linei(buffer, w, p1, p2, color);
    draw_linei(buffer, w, p2, p3, color);
    draw_linei(buffer, w, p3, p1, color);
    draw_cirlcei(buffer, w, p1, 5, RED, mode);
    draw_cirlcei(buffer, w, p2, 5, RED, mode);
    draw_cirlcei(buffer, w, p3, 5, RED, mode);
  }
}

void draw_cirlcei(u32 *buffer, int w, v2i pos, int r, u32 color, u32 mode) {

  if (mode == WIREFRAME) {
    int x = 0;
    int y = -r;
    int d = -r;

    while (x < -y) {
      if (d > 0) {
        y += 1;
        d += 2 * (x + y) + 1;
      } else {
        d += 2 * x + 1;
      }
      set_pixel(buffer, w, (v2i){pos.x + x, pos.y + y}, color);
      set_pixel(buffer, w, (v2i){pos.x - x, pos.y + y}, color);
      set_pixel(buffer, w, (v2i){pos.x + x, pos.y - y}, color);
      set_pixel(buffer, w, (v2i){pos.x - x, pos.y - y}, color);
      set_pixel(buffer, w, (v2i){pos.x + y, pos.y + x}, color);
      set_pixel(buffer, w, (v2i){pos.x + y, pos.y - x}, color);
      set_pixel(buffer, w, (v2i){pos.x - y, pos.y + x}, color);
      set_pixel(buffer, w, (v2i){pos.x - y, pos.y - x}, color);

      x += 1;
    }
  }
}
