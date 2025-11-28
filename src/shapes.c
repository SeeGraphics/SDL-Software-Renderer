#include "shapes.h"
#include "colors.h"
#include "render.h"
#include "types.h"
#include "utils.h"
#include <math.h>

void draw_triangle(u32 *buffer, int w, int h, v2i p1, v2i p2, v2i p3, u32 color,
                   u32 mode) {
  if (mode == WIREFRAME) {
    draw_linei(buffer, w, p1, p2, color);
    draw_linei(buffer, w, p2, p3, color);
    draw_linei(buffer, w, p3, p1, color);
  }
  if (mode == FILLED) {
    sort_by_y(&p1, &p2, &p3);
    if (p1.y == p3.y) {
      return;
    }
    int y_start = p1.y;
    int y_end = p3.y;
    for (int y = y_start; y < y_end; y++) {
      // left & right x-borders
      float xa, xb;
      if (y < p2.y) {
        // upper part (p1 -> p3 && p1 -> p2)
        xa = p1.x + (float)(p3.x - p1.x) * (y - p1.y) / (float)(p3.y - p1.y);
        xb = p1.x + (float)(p2.x - p1.x) * (y - p1.y) / (float)(p2.y - p1.y);
      } else {
        // lower part (p1 -> p3 && p2 -> p3)
        xa = p1.x + (float)(p3.x - p1.x) * (y - p1.y) / (float)(p3.y - p1.y);
        xb = p2.x + (float)(p3.x - p2.x) * (y - p2.y) / (float)(p3.y - p2.y);
      }
      // if there not sorted just switch them
      if (xa > xb) {
        float t = xa;
        xa = xb;
        xb = t;
      }
      // make sure we start filling from closest pixel
      int xl = (int)ceilf(xa);
      int xr = (int)floorf(xb);
      // clamp to buffer bounds
      if (xl < 0)
        xl = 0;
      if (xr >= w)
        xr = w - 1;
      if (y < 0 || y >= h)
        continue;

      // fill out
      for (int x = xl; x <= xr; x++)
        set_pixel(buffer, w, (v2i){x, y}, color);
    }
  }
}

void draw_triangle_dots(u32 *buffer, int w, int h, v2i p1, v2i p2, v2i p3,
                        u32 color, u32 mode) {
  if (mode == WIREFRAME) {
    draw_linei(buffer, w, p1, p2, color);
    draw_linei(buffer, w, p2, p3, color);
    draw_linei(buffer, w, p3, p1, color);
    draw_cirlcei(buffer, w, p1, 5, RED);
    draw_cirlcei(buffer, w, p2, 5, RED);
    draw_cirlcei(buffer, w, p3, 5, RED);
  }
  if (mode == FILLED) {
    sort_by_y(&p1, &p2, &p3);
    if (p1.y == p3.y) {
      return;
    }
    int y_start = p1.y;
    int y_end = p3.y;
    for (int y = y_start; y < y_end; y++) {
      // which edges are active
      float xa, xb;
      if (y < p2.y) {
        // upper part (p1 -> p3 && p1 -> p2)
        xa = p1.x + (float)(p3.x - p1.x) * (y - p1.y) / (float)(p3.y - p1.y);
        xb = p1.x + (float)(p2.x - p1.x) * (y - p1.y) / (float)(p2.y - p1.y);
      } else {
        // lower part (p1 -> p3 && p2 -> p3)
        xa = p1.x + (float)(p3.x - p1.x) * (y - p1.y) / (float)(p3.y - p1.y);
        xb = p2.x + (float)(p3.x - p2.x) * (y - p2.y) / (float)(p3.y - p2.y);
      }
      if (xa > xb) {
        float t = xa;
        xa = xb;
        xb = t;
      }
      int xl = (int)ceilf(xa);
      int xr = (int)floorf(xb);
      // clamp to buffer bounds
      if (xl < 0)
        xl = 0;
      if (xr >= w)
        xr = w - 1;
      if (y < 0 || y >= h)
        continue;

      // fill out
      for (int x = xl; x <= xr; x++)
        set_pixel(buffer, w, (v2i){x, y}, color);
    }
    draw_cirlcei(buffer, w, p1, 5, RED);
    draw_cirlcei(buffer, w, p2, 5, RED);
    draw_cirlcei(buffer, w, p3, 5, RED);
  }
}

static inline float edge_func(v2i a, v2i b, float x, float y) {
  return (y - (float)a.y) * ((float)b.x - (float)a.x) -
         (x - (float)a.x) * ((float)b.y - (float)a.y);
}

void draw_textured_triangle(u32 *buffer, float *depth, int w, int h, Texture *tex,
                            VertexPC v0, VertexPC v1, VertexPC v2) {
  // Bounding box
  int min_x = fminf(fminf(v0.pos.x, v1.pos.x), v2.pos.x);
  int max_x = fmaxf(fmaxf(v0.pos.x, v1.pos.x), v2.pos.x);
  int min_y = fminf(fminf(v0.pos.y, v1.pos.y), v2.pos.y);
  int max_y = fmaxf(fmaxf(v0.pos.y, v1.pos.y), v2.pos.y);

  if (max_x < 0 || max_y < 0 || min_x >= w || min_y >= h) {
    return;
  }

  if (min_x < 0)
    min_x = 0;
  if (min_y < 0)
    min_y = 0;
  if (max_x >= w)
    max_x = w - 1;
  if (max_y >= h)
    max_y = h - 1;

  float area = edge_func(v0.pos, v1.pos, (float)v2.pos.x, (float)v2.pos.y);
  if (area == 0.0f) {
    return;
  }
  float inv_area = 1.0f / area;

  for (int y = min_y; y <= max_y; y++) {
    for (int x = min_x; x <= max_x; x++) {
      float px = (float)x + 0.5f;
      float py = (float)y + 0.5f;
      float w0 = edge_func(v1.pos, v2.pos, px, py) * inv_area;
      float w1 = edge_func(v2.pos, v0.pos, px, py) * inv_area;
      float w2 = edge_func(v0.pos, v1.pos, px, py) * inv_area;

      if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) {
        continue;
      }

      float inv_w_interp = w0 * v0.inv_w + w1 * v1.inv_w + w2 * v2.inv_w;
      if (inv_w_interp == 0.0f) {
        continue;
      }

      float u_over_w =
          w0 * (v0.uv.x * v0.inv_w) + w1 * (v1.uv.x * v1.inv_w) + w2 * (v2.uv.x * v2.inv_w);
      float v_over_w =
          w0 * (v0.uv.y * v0.inv_w) + w1 * (v1.uv.y * v1.inv_w) + w2 * (v2.uv.y * v2.inv_w);
      float u = u_over_w / inv_w_interp;
      float v = v_over_w / inv_w_interp;

      float depth_interp = w0 * v0.depth + w1 * v1.depth + w2 * v2.depth;
      int idx = y * w + x;
      if (depth_interp >= depth[idx]) {
        continue;
      }
      depth[idx] = depth_interp;

      if (u < 0.0f)
        u = 0.0f;
      if (u > 1.0f)
        u = 1.0f;
      if (v < 0.0f)
        v = 0.0f;
      if (v > 1.0f)
        v = 1.0f;

      int tx = (int)(u * (float)(tex->w - 1));
      int ty = (int)(v * (float)(tex->h - 1));
      u32 sample = tex->pixels[ty * tex->w + tx];
      set_pixel(buffer, w, (v2i){x, y}, sample);
    }
  }
}

void draw_cirlcei(u32 *buffer, int w, v2i pos, int r, u32 color) {
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
