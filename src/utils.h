#pragma once

#include "types.h"
#include <SDL2/SDL.h>

void buffer_reallocate(u32 **buffer, u32 w, u32 h, int size);
void texture_recreate(SDL_Texture **tex, SDL_Renderer *r, u32 w, u32 h);
void pitch_update(u32 *pitch, u32 w, u32 size);
