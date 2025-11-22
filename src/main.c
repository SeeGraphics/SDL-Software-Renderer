#include "render.h"
#include "types.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  SDL_Texture *texture;
  u32 *buffer;
  u32 pitch;
  bool quit;
} Game;

int main(void) {
  Game game = {0};

  u32 window_width = 800;
  u32 window_height = 600;
  const char *title = "A: Hello Window";

  game.pitch = window_width * sizeof(u32);

  // test dot
  v2i pos = {(int)window_width / 2, (int)window_height / 2};

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Failed to Initialize SDL: %s\n", SDL_GetError());
  }

  game.window = SDL_CreateWindow(
      title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width,
      window_height, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_RESIZABLE);
  if (game.window == NULL) {
    SDL_Log("Failed to create Window: %s\n", SDL_GetError());
    SDL_Quit();
  }
  SDL_RaiseWindow(game.window); // immediately focus window

  game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
  if (game.renderer == NULL) {
    SDL_Log("Failed to create Renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(game.window);
    SDL_Quit();
  }

  game.texture = SDL_CreateTexture(game.renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, window_width,
                                   window_height);

  game.buffer = malloc(window_width * window_height * sizeof(u32));

  while (!game.quit) {
    while (SDL_PollEvent(&game.event)) {
      switch (game.event.type) {
      case SDL_QUIT:
        game.quit = true;
        break;
      case SDL_KEYDOWN:
        if (game.event.key.keysym.sym == SDLK_ESCAPE) {
          game.quit = true;
          break;
        case SDL_WINDOWEVENT:
          if (game.event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            window_width = game.event.window.data1;
            window_height = game.event.window.data2;

            // update & reallocate
            texture_recreate(&game.texture, game.renderer, window_width,
                             window_height);
            buffer_reallocate(&game.buffer, window_width, window_height,
                              sizeof(u32));
            pitch_update(&game.pitch, window_width, sizeof(u32));
            SDL_RenderSetViewport(game.renderer, NULL);
            break;
          }
        }
      }
    }

    // update test dot position
    pos.x = (int)window_width / 2;
    pos.y = (int)window_height / 2;

    // black background
    memset(game.buffer, 0, window_width * window_height * sizeof(u32));
    set_pixel(game.buffer, window_width, pos, 0xFFFFFFFF); // test dot

    SDL_UpdateTexture(game.texture, NULL, game.buffer, game.pitch);
    SDL_RenderClear(game.renderer);
    SDL_RenderCopy(game.renderer, game.texture, NULL, NULL);
    SDL_RenderPresent(game.renderer);
  }
  free(game.buffer);
  SDL_DestroyTexture(game.texture);
  SDL_DestroyRenderer(game.renderer);
  SDL_DestroyWindow(game.window);
  SDL_Quit();

  return 0;
}
