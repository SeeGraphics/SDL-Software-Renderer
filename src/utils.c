#include "utils.h"

void buffer_reallocate(u32 **buffer, u32 w, u32 h, int size) {
  free(*buffer);
  *buffer = malloc(w * h * size);
}

void texture_recreate(SDL_Texture **tex, SDL_Renderer *r, u32 w, u32 h) {
  if (*tex) {
    SDL_DestroyTexture(*tex);
  }
  *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                           SDL_TEXTUREACCESS_STREAMING, w, h);
}

void pitch_update(u32 *pitch, u32 w, u32 size) { *pitch = w * size; }

void clamp_v2i(v2i *pos, int min1, int max1, int min2, int max2, int r) {
  if (pos->x < min1 + r)
    pos->x = min1 + r;
  if (pos->x > max1 - r)
    pos->x = max1 - r;
  if (pos->y < min2 + r)
    pos->y = min2 + r;
  if (pos->y > max2 - r)
    pos->y = max2 - r;
}
