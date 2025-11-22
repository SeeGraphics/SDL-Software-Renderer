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
