#pragma once

#include "types.h"
#include <SDL2/SDL.h>

void buffer_reallocate(u32 **buffer, u32 w, u32 h, int size);
void texture_recreate(SDL_Texture **tex, SDL_Renderer *r, u32 w, u32 h);
void pitch_update(u32 *pitch, u32 w, u32 size);
void swap_v2i(v2i *a, v2i *b);
void sort_by_y(v2i *p1, v2i *p2, v2i *p3);
void clamp_v2i(v2i *pos, int min1, int max1, int min2, int max2, int r);
