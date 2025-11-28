#include "colors.h"
#include "math.h"
#include "render.h"
#include "shapes.h"
#include "text.h"
#include "types.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

typedef struct {
  u32 window_w;
  u32 window_h;
  u32 render_w;
  u32 render_h;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  SDL_Texture *texture;
  u32 *buffer;
  float *depth;
  u32 pitch;
  bool quit;
  bool mouse_grabbed;
} Game;

typedef struct {
  v3f pos;
  float yaw;
  float pitch;
} Camera;

static v3f camera_forward(const Camera *cam) {
  float cy = cosf(cam->yaw);
  float sy = sinf(cam->yaw);
  float cp = cosf(cam->pitch);
  float sp = sinf(cam->pitch);
  return v3_normalize((v3f){sy * cp, sp, -cy * cp});
}

static void clear_depth(float *depth, size_t count) {
  for (size_t i = 0; i < count; i++) {
    depth[i] = 1.0f;
  }
}

typedef struct {
  v3f view_pos;
  v2f uv;
} ClipVert;

static bool project_vertex(const ClipVert *cv, const mat4 *proj, int render_w,
                           int render_h, VertexPC *out, int *mask_out) {
  v4f clip = mat4_mul_v4(*proj,
                         (v4f){cv->view_pos.x, cv->view_pos.y, cv->view_pos.z,
                               1.0f});
  if (clip.w == 0.0f) {
    return false;
  }

  int mask = 0;
  if (clip.x < -clip.w)
    mask |= 1;
  if (clip.x > clip.w)
    mask |= 2;
  if (clip.y < -clip.w)
    mask |= 4;
  if (clip.y > clip.w)
    mask |= 8;
  if (clip.z < 0.0f)
    mask |= 16;
  if (clip.z > clip.w)
    mask |= 32;

  float inv_w = 1.0f / clip.w;
  v3f ndc = {clip.x * inv_w, clip.y * inv_w, clip.z * inv_w};
  out->pos = norm_to_screen((v2f){ndc.x, ndc.y}, render_w, render_h);
  out->uv = cv->uv;
  out->inv_w = inv_w;
  out->depth = 0.5f * (ndc.z + 1.0f);
  *mask_out = mask;
  return true;
}

int main(void) {
  Game game = {.window_w = 800, .window_h = 600};
  Camera camera = {.pos = {0.0f, 0.0f, 2.0f}, .yaw = 0.0f, .pitch = 0.0f};
  const float mouse_sens = 0.0025f;
  const int render_scale = 2; // 1/render_scale size; 2 => quarter pixels

  static const Vertex3D cube_vertices[] = {
      // Front (-Z)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},
      // Back (+Z)
      {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
      // Left (-X)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
      // Right (+X)
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},
      // Top (+Y)
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
      // Bottom (-Y)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
  };

  static const int cube_indices[][3] = {
      {0, 1, 2},   {0, 2, 3},   // front
      {4, 5, 6},   {4, 6, 7},   // back
      {8, 9, 10},  {8, 10, 11}, // left
      {12, 13, 14}, {12, 14, 15}, // right
      {16, 17, 18}, {16, 18, 19}, // top
      {20, 21, 22}, {20, 22, 23}  // bottom
  };
  const int cube_vertex_count =
      (int)(sizeof(cube_vertices) / sizeof(cube_vertices[0]));
  const int cube_triangle_count =
      (int)(sizeof(cube_indices) / sizeof(cube_indices[0]));

  const char *texture_path = "assets/brick.png";
  Texture texture = {0};

  const char *title = "A: Hello Window";
  game.render_w = game.window_w / render_scale;
  game.render_h = game.window_h / render_scale;
  if (game.render_w == 0)
    game.render_w = 1;
  if (game.render_h == 0)
    game.render_h = 1;
  game.pitch = game.render_w * sizeof(u32);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Failed to Initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    SDL_Log("Failed to init SDL_image: %s\n", IMG_GetError());
    SDL_Quit();
    return 1;
  }

  if (!texture_load(&texture, texture_path)) {
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  game.window = SDL_CreateWindow(
      title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, game.window_w,
      game.window_h, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_RESIZABLE);
  if (game.window == NULL) {
    SDL_Log("Failed to create Window: %s\n", SDL_GetError());
    texture_destroy(&texture);
    IMG_Quit();
    SDL_Quit();
    return 1;
  }
  SDL_RaiseWindow(game.window); // immediately focus window

  game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
  if (game.renderer == NULL) {
    SDL_Log("Failed to create Renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(game.window);
    texture_destroy(&texture);
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  game.texture = SDL_CreateTexture(game.renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, game.render_w,
                                   game.render_h);

  game.buffer = malloc(game.render_w * game.render_h * sizeof(u32));
  game.depth = malloc(game.render_w * game.render_h * sizeof(float));
  SDL_SetRelativeMouseMode(SDL_TRUE);
  game.mouse_grabbed = true;
  float fps = 0.0f;

  Uint32 last_ticks = SDL_GetTicks();

  typedef struct {
    v2i screen;
    v2f uv;
    v3f view_pos;
    float inv_w;
    float depth;
    int clip_mask;
    bool depth_ok;
  } CachedVertex;
  CachedVertex cached[cube_vertex_count];

  const float near_plane = 0.1f;

  while (!game.quit) {
    Uint32 now = SDL_GetTicks();
    float dt = (now - last_ticks) / 1000.0f;
    last_ticks = now;
    if (dt > 0.0f) {
      float inst = 1.0f / dt;
      fps = fps * 0.9f + inst * 0.1f;
    }

    while (SDL_PollEvent(&game.event)) {
      switch (game.event.type) {
      case SDL_QUIT:
        game.quit = true;
        break;
      case SDL_WINDOWEVENT:
        if (game.event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          game.window_w = (u32)game.event.window.data1;
          game.window_h = (u32)game.event.window.data2;
          game.render_w = game.window_w / render_scale;
          game.render_h = game.window_h / render_scale;
          if (game.render_w == 0)
            game.render_w = 1;
          if (game.render_h == 0)
            game.render_h = 1;
          buffer_reallocate(&game.buffer, game.render_w, game.render_h,
                            sizeof(u32));
          if (game.depth) {
            free(game.depth);
          }
          game.depth = malloc(game.render_w * game.render_h * sizeof(float));
          pitch_update(&game.pitch, game.render_w, sizeof(u32));
          texture_recreate(&game.texture, game.renderer, game.render_w,
                           game.render_h);
        }
        break;
      case SDL_MOUSEMOTION:
        camera.yaw += (float)game.event.motion.xrel * mouse_sens;
        camera.pitch -= (float)game.event.motion.yrel * mouse_sens;
        break;
      case SDL_KEYDOWN:
        if (game.event.key.keysym.sym == SDLK_ESCAPE) {
          game.quit = true;
        }
        if (game.event.key.keysym.sym == SDLK_q) {
          game.mouse_grabbed = !game.mouse_grabbed;
          SDL_SetRelativeMouseMode(game.mouse_grabbed ? SDL_TRUE : SDL_FALSE);
          SDL_ShowCursor(game.mouse_grabbed ? SDL_DISABLE : SDL_ENABLE);
        }
        break;
      default:
        break;
      }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    v3f forward = camera_forward(&camera);
    v3f world_up = {0.0f, 1.0f, 0.0f};
    v3f right = v3_normalize(v3_cross(forward, world_up));

    float move_speed = 2.5f * dt;
    if (state[SDL_SCANCODE_W]) {
      camera.pos = v3_add(camera.pos, v3_scale(forward, move_speed));
    }
    if (state[SDL_SCANCODE_S]) {
      camera.pos = v3_sub(camera.pos, v3_scale(forward, move_speed));
    }
    if (state[SDL_SCANCODE_A]) {
      camera.pos = v3_sub(camera.pos, v3_scale(right, move_speed));
    }
    if (state[SDL_SCANCODE_D]) {
      camera.pos = v3_add(camera.pos, v3_scale(right, move_speed));
    }
    if (state[SDL_SCANCODE_SPACE]) {
      camera.pos.y += move_speed;
    }
    if (state[SDL_SCANCODE_LCTRL]) {
      camera.pos.y -= move_speed;
    }

    float look_speed = 1.5f * dt;
    if (state[SDL_SCANCODE_LEFT]) {
      camera.yaw -= look_speed;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
      camera.yaw += look_speed;
    }
    if (state[SDL_SCANCODE_UP]) {
      camera.pitch += look_speed;
    }
    if (state[SDL_SCANCODE_DOWN]) {
      camera.pitch -= look_speed;
    }
    float max_pitch = (float)M_PI_2 - 0.1f;
    if (camera.pitch > max_pitch)
      camera.pitch = max_pitch;
    if (camera.pitch < -max_pitch)
      camera.pitch = -max_pitch;

    memset(game.buffer, 0, game.render_w * game.render_h * sizeof(u32));
    clear_depth(game.depth, (size_t)game.render_w * (size_t)game.render_h);

    float aspect = (float)game.render_w / (float)game.render_h;
    float angle = (float)now * 0.001f;
    mat4 model =
        mat4_mul(mat4_rotate_y(angle), mat4_rotate_x(angle * 0.5f));
    mat4 view = mat4_look_at(
        camera.pos, v3_add(camera.pos, camera_forward(&camera)), world_up);
    mat4 proj = mat4_perspective((float)M_PI / 3.0f, aspect, near_plane, 100.0f);
    mat4 mv = mat4_mul(view, model);

    for (int i = 0; i < cube_vertex_count; i++) {
      v4f world = {cube_vertices[i].pos.x, cube_vertices[i].pos.y,
                   cube_vertices[i].pos.z, 1.0f};
      v4f view_pos4 = mat4_mul_v4(mv, world);
      v4f clip = mat4_mul_v4(proj, view_pos4);

      cached[i].uv = cube_vertices[i].uv;
      cached[i].view_pos = (v3f){view_pos4.x, view_pos4.y, view_pos4.z};

      int mask = 0;
      if (clip.w == 0.0f) {
        cached[i].clip_mask = 0x3F; // force cull
        cached[i].depth_ok = false;
        continue;
      }
      if (clip.x < -clip.w)
        mask |= 1;
      if (clip.x > clip.w)
        mask |= 2;
      if (clip.y < -clip.w)
        mask |= 4;
      if (clip.y > clip.w)
        mask |= 8;
      if (clip.z < 0.0f)
        mask |= 16;
      if (clip.z > clip.w)
        mask |= 32;
      cached[i].clip_mask = mask;

      float inv_w = 1.0f / clip.w;
      cached[i].inv_w = inv_w;
      v3f ndc = {clip.x * inv_w, clip.y * inv_w, clip.z * inv_w};
      cached[i].depth_ok = ndc.z >= 0.0f && ndc.z <= 1.0f;
      cached[i].screen =
          norm_to_screen((v2f){ndc.x, ndc.y}, game.render_w, game.render_h);
      cached[i].depth = 0.5f * (ndc.z + 1.0f);
    }

    for (int tri_idx = 0; tri_idx < cube_triangle_count; tri_idx++) {
      const int i0 = cube_indices[tri_idx][0];
      const int i1 = cube_indices[tri_idx][1];
      const int i2 = cube_indices[tri_idx][2];
      const CachedVertex *v0 = &cached[i0];
      const CachedVertex *v1 = &cached[i1];
      const CachedVertex *v2 = &cached[i2];

      if ((v0->clip_mask & v1->clip_mask & v2->clip_mask) != 0) {
        continue; // frustum culled
      }

      bool near_in[3] = {v0->view_pos.z <= -near_plane,
                         v1->view_pos.z <= -near_plane,
                         v2->view_pos.z <= -near_plane};
      bool needs_clip = !(near_in[0] && near_in[1] && near_in[2]);

      if (!needs_clip) {
        if (!v0->depth_ok || !v1->depth_ok || !v2->depth_ok) {
          continue;
        }
        v3f edge1 = v3_sub(v1->view_pos, v0->view_pos);
        v3f edge2 = v3_sub(v2->view_pos, v0->view_pos);
        v3f normal = v3_cross(edge1, edge2);
        if (normal.z >= 0.0f) {
          continue; // backface
        }

        VertexPC pv[3] = {
            {.pos = v0->screen, .uv = v0->uv, .inv_w = v0->inv_w, .depth = v0->depth},
            {.pos = v1->screen, .uv = v1->uv, .inv_w = v1->inv_w, .depth = v1->depth},
            {.pos = v2->screen, .uv = v2->uv, .inv_w = v2->inv_w, .depth = v2->depth},
        };

        draw_textured_triangle(game.buffer, game.depth, game.render_w,
                               game.render_h, &texture, pv[0], pv[1], pv[2]);
      } else {
        ClipVert in_poly[4] = {
            {.view_pos = v0->view_pos, .uv = v0->uv},
            {.view_pos = v1->view_pos, .uv = v1->uv},
            {.view_pos = v2->view_pos, .uv = v2->uv},
        };
        int in_count = 3;
        ClipVert out_poly[4];
        int out_count = 0;

        for (int i = 0; i < in_count; i++) {
          ClipVert a = in_poly[i];
          ClipVert b = in_poly[(i + 1) % in_count];
          bool a_in = a.view_pos.z <= -near_plane;
          bool b_in = b.view_pos.z <= -near_plane;

          if (a_in && b_in) {
            out_poly[out_count++] = b;
          } else if (a_in && !b_in) {
            float t = (-near_plane - a.view_pos.z) /
                      (b.view_pos.z - a.view_pos.z);
            ClipVert inter = {
                .view_pos = {a.view_pos.x + (b.view_pos.x - a.view_pos.x) * t,
                             a.view_pos.y + (b.view_pos.y - a.view_pos.y) * t,
                             -near_plane},
                .uv = {a.uv.x + (b.uv.x - a.uv.x) * t,
                       a.uv.y + (b.uv.y - a.uv.y) * t}};
            out_poly[out_count++] = inter;
          } else if (!a_in && b_in) {
            float t = (-near_plane - a.view_pos.z) /
                      (b.view_pos.z - a.view_pos.z);
            ClipVert inter = {
                .view_pos = {a.view_pos.x + (b.view_pos.x - a.view_pos.x) * t,
                             a.view_pos.y + (b.view_pos.y - a.view_pos.y) * t,
                             -near_plane},
                .uv = {a.uv.x + (b.uv.x - a.uv.x) * t,
                       a.uv.y + (b.uv.y - a.uv.y) * t}};
            out_poly[out_count++] = inter;
            out_poly[out_count++] = b;
          }
        }

        if (out_count < 3) {
          continue;
        }

        int tri_sets[2][3] = {{0, 1, 2}, {0, 2, 3}};
        int tri_total = (out_count == 4) ? 2 : 1;

        for (int t = 0; t < tri_total; t++) {
          ClipVert *a = &out_poly[tri_sets[t][0]];
          ClipVert *b = &out_poly[tri_sets[t][1]];
          ClipVert *c = &out_poly[tri_sets[t][2]];

          v3f edge1 = v3_sub(b->view_pos, a->view_pos);
          v3f edge2 = v3_sub(c->view_pos, a->view_pos);
          v3f normal = v3_cross(edge1, edge2);
          if (normal.z >= 0.0f) {
            continue; // backface
          }

          VertexPC pv[3];
          int masks[3];
          if (!project_vertex(a, &proj, game.render_w, game.render_h, &pv[0],
                              &masks[0]) ||
              !project_vertex(b, &proj, game.render_w, game.render_h, &pv[1],
                              &masks[1]) ||
              !project_vertex(c, &proj, game.render_w, game.render_h, &pv[2],
                              &masks[2])) {
            continue;
          }
          if ((masks[0] & masks[1] & masks[2]) != 0) {
            continue;
          }

          draw_textured_triangle(game.buffer, game.depth, game.render_w,
                                 game.render_h, &texture, pv[0], pv[1], pv[2]);
        }
      }
    }

    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS %d", (int)(fps + 0.5f));
    draw_text(game.buffer, game.render_w, (v2i){5, 5}, fps_text, WHITE);

  SDL_UpdateTexture(game.texture, NULL, game.buffer, game.pitch);
  SDL_RenderClear(game.renderer);
  SDL_Rect dest = {0, 0, (int)game.window_w, (int)game.window_h};
  SDL_RenderCopy(game.renderer, game.texture, NULL, &dest);
  SDL_RenderPresent(game.renderer);
  }
  free(game.buffer);
  free(game.depth);
  texture_destroy(&texture);
  IMG_Quit();
  SDL_DestroyTexture(game.texture);
  SDL_DestroyRenderer(game.renderer);
  SDL_DestroyWindow(game.window);
  SDL_Quit();

  return 0;
}
