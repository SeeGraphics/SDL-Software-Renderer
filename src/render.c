#include "render.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

v2i norm_to_screen(v2f norm, int w, int h) {
  v2i screen;
  screen.x = (int)((norm.x * 0.5f + 0.5f) * (float)(w - 1));
  screen.y = (int)((-norm.y * 0.5f + 0.5f) * (float)(h - 1));
  return screen;
}

v2f screen_to_norm(v2i screen, int w, int h) {
  v2f norm;
  norm.x = 2.0f * ((float)screen.x / (float)(w - 1)) - 1.0f;
  norm.y = 1.0f - 2.0f * ((float)screen.y / (float)(h - 1));
  return norm;
}

bool texture_load(Texture *tex, const char *path) {
  tex->pixels = NULL;
  tex->w = 0;
  tex->h = 0;

  SDL_Surface *loaded = IMG_Load(path);
  if (!loaded) {
    SDL_Log("Failed to load texture '%s': %s", path, IMG_GetError());
    return false;
  }

  SDL_Surface *converted =
      SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ARGB8888, 0);
  SDL_FreeSurface(loaded);
  if (!converted) {
    SDL_Log("Failed to convert texture '%s': %s", path, SDL_GetError());
    return false;
  }

  tex->w = converted->w;
  tex->h = converted->h;
  size_t size = (size_t)tex->w * (size_t)tex->h;
  tex->pixels = malloc(size * sizeof(u32));
  if (!tex->pixels) {
    SDL_FreeSurface(converted);
    SDL_Log("Failed to allocate texture memory for '%s'", path);
    return false;
  }
  memcpy(tex->pixels, converted->pixels, size * sizeof(u32));
  SDL_FreeSurface(converted);
  return true;
}

void texture_destroy(Texture *tex) {
  if (tex->pixels) {
    free(tex->pixels);
    tex->pixels = NULL;
  }
  tex->w = 0;
  tex->h = 0;
}

void set_pixel(u32 *buffer, int w, v2i pos, u32 color) {
  buffer[pos.y * w + pos.x] = color;
}

void draw_linei(u32 *buffer, int w, int h, v2i p1, v2i p2, u32 color) {
  int dx = abs(p2.x - p1.x);
  int dy = abs(p2.y - p1.y);
  int sx = (p1.x < p2.x) ? 1 : -1;
  int sy = (p1.y < p2.y) ? 1 : -1;
  int d = dx - dy;

  for (;;) {
    if (p1.x >= 0 && p1.x < w && p1.y >= 0 && p1.y < h) {
      set_pixel(buffer, w, p1, color);
    }
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
