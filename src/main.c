#include "colors.h"
#include "render.h"
#include "shapes.h"
#include "types.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define RENDER_WIDTH 800
#define RENDER_HEIGHT 600

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

  v2i p1 = {100, 100};
  v2i p2 = {300, 300};
  v2i p3 = {180, 500};

  const char *title = "A: Hello Window";
  game.pitch = RENDER_WIDTH * sizeof(u32);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Failed to Initialize SDL: %s\n", SDL_GetError());
  }

  game.window = SDL_CreateWindow(
      title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, RENDER_WIDTH,
      RENDER_HEIGHT, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_RESIZABLE);
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
  SDL_RenderSetLogicalSize(game.renderer, RENDER_WIDTH, RENDER_HEIGHT);

  game.texture = SDL_CreateTexture(game.renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, RENDER_WIDTH,
                                   RENDER_HEIGHT);

  game.buffer = malloc(RENDER_WIDTH * RENDER_HEIGHT * sizeof(u32));

  while (!game.quit) {
    while (SDL_PollEvent(&game.event)) {
      switch (game.event.type) {
      case SDL_QUIT:
        game.quit = true;
        break;
        // TODO: add mousevent that prints out coordinates, so testing is easier
        // TODO: also try and figure out how normalizing coordinates work
        // so that all coordinates are in -1.0 - 1.0 space
      case SDL_KEYDOWN:
        if (game.event.key.keysym.sym == SDLK_ESCAPE) {
          game.quit = true;
          break;
        }
      default:
        break;
      }
    }

    // black background
    memset(game.buffer, 0, RENDER_WIDTH * RENDER_HEIGHT * sizeof(u32));

    // draw
    draw_triangle(game.buffer, RENDER_WIDTH, p1, p2, p3, WHITE);

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
