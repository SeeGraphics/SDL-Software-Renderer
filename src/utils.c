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

void swap_v2i(v2i *a, v2i *b) {
  v2i temp = *a;
  *a = *b;
  *b = temp;
}

void pitch_update(u32 *pitch, u32 w, u32 size) { *pitch = w * size; }

void sort_by_y(v2i *p1, v2i *p2, v2i *p3) {
  if (p1->y > p2->y)
    swap_v2i(p1, p2);
  if (p2->y > p3->y)
    swap_v2i(p2, p3);
  if (p1->y > p2->y) // making sure that p1 is actually the smallest
    swap_v2i(p1, p2);
}

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
