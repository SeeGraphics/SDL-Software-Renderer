#include "obj_loader.h"
#include "render.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char path[256];
  ObjMaterial material;
} MtlEntry;

static void model_reset(ObjModel *model) {
  if (!model) {
    return;
  }
  model->faces = NULL;
  model->face_count = 0;
  model->materials = NULL;
  model->material_count = 0;
  model->bounds_min = (v3f){0};
  model->bounds_max = (v3f){0};
  model->has_bounds = false;
}

static bool ensure_capacity(void **data, int *cap, int needed, size_t elem) {
  if (needed <= *cap) {
    return true;
  }
  int new_cap = (*cap == 0) ? 64 : *cap * 2;
  while (new_cap < needed) {
    new_cap *= 2;
  }
  void *tmp = realloc(*data, (size_t)new_cap * elem);
  if (!tmp) {
    return false;
  }
  *data = tmp;
  *cap = new_cap;
  return true;
}

static void dirname_of(const char *path, char *out, size_t out_size) {
  const char *slash = strrchr(path, '/');
  if (!slash) {
    out[0] = '\0';
    return;
  }
  size_t len = (size_t)(slash - path);
  if (len >= out_size) {
    len = out_size - 1;
  }
  memcpy(out, path, len);
  out[len] = '\0';
}

static void join_path(char *out, size_t out_size, const char *base,
                      const char *rel) {
  if (rel[0] == '/' || (strlen(rel) > 1 && rel[1] == ':')) {
    snprintf(out, out_size, "%s", rel);
    return;
  }
  if (base[0] == '\0') {
    snprintf(out, out_size, "%s", rel);
    return;
  }
  snprintf(out, out_size, "%s/%s", base, rel);
}

static ObjMaterial *find_material(ObjModel *model, const char *name) {
  for (int i = 0; i < model->material_count; i++) {
    if (strcmp(model->materials[i].name, name) == 0) {
      return &model->materials[i];
    }
  }
  return NULL;
}

static bool parse_mtl(const char *mtl_path, ObjModel *model, int *mat_cap) {
  FILE *f = fopen(mtl_path, "r");
  if (!f) {
    return false;
  }

  char base_dir[256];
  dirname_of(mtl_path, base_dir, sizeof(base_dir));

  ObjMaterial *current = NULL;
  char line[512];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#') {
      continue;
    }
    if (strncmp(line, "newmtl", 6) == 0) {
      char name[64];
      if (sscanf(line + 6, "%63s", name) == 1) {
        if (!ensure_capacity((void **)&model->materials, mat_cap,
                             model->material_count + 1,
                             sizeof(ObjMaterial))) {
          fclose(f);
          return false;
        }
        current = &model->materials[model->material_count++];
        memset(current, 0, sizeof(*current));
        strncpy(current->name, name, sizeof(current->name) - 1);
      }
      continue;
    }
    if (strncmp(line, "map_Kd", 6) == 0 && current) {
      char tex_rel[256];
      if (sscanf(line + 6, "%255s", tex_rel) == 1) {
        char tex_path[512];
        join_path(tex_path, sizeof(tex_path), base_dir, tex_rel);
        if (texture_load(&current->diffuse, tex_path)) {
          current->has_diffuse = true;
        }
      }
    }
  }

  fclose(f);
  return true;
}

static bool parse_index_triplet(const char *token, int *vi, int *ti, int *ni) {
  *vi = *ti = *ni = 0;
  const char *p = token;
  if (!*p) {
    return false;
  }
  char *end = NULL;
  *vi = (int)strtol(p, &end, 10);
  if (end == p) {
    return false;
  }
  p = end;
  if (*p == '/') {
    p++;
    if (*p != '/' && *p != '\0') {
      *ti = (int)strtol(p, &end, 10);
      p = end;
    }
    if (*p == '/') {
      p++;
      if (*p != '\0') {
        *ni = (int)strtol(p, &end, 10);
      }
    }
  }
  return true;
}

bool obj_model_load(const char *obj_path, ObjModel *out) {
  if (!obj_path || !out) {
    return false;
  }

  model_reset(out);

  FILE *f = fopen(obj_path, "r");
  if (!f) {
    return false;
  }

  char base_dir[256];
  dirname_of(obj_path, base_dir, sizeof(base_dir));

  v3f *positions = NULL;
  int pos_count = 0, pos_cap = 0;
  v2f *uvs = NULL;
  int uv_count = 0, uv_cap = 0;
  ObjFace *faces = NULL;
  int face_count = 0, face_cap = 0;
  int material_cap = 0;
  bool success = true;

  ObjMaterial *current_mat = NULL;

  char line[512];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#') {
      continue;
    }
    if (strncmp(line, "mtllib", 6) == 0) {
      char mtl_rel[256];
      if (sscanf(line + 6, "%255s", mtl_rel) == 1) {
        char mtl_path[512];
        join_path(mtl_path, sizeof(mtl_path), base_dir, mtl_rel);
        parse_mtl(mtl_path, out, &material_cap);
      }
      continue;
    }
    if (strncmp(line, "usemtl", 6) == 0) {
      char mat_name[64];
      if (sscanf(line + 6, "%63s", mat_name) == 1) {
        current_mat = find_material(out, mat_name);
      }
      continue;
    }
    if (line[0] == 'v' && isspace((unsigned char)line[1])) {
      float x, y, z;
      if (sscanf(line + 1, "%f %f %f", &x, &y, &z) == 3) {
        if (!ensure_capacity((void **)&positions, &pos_cap, pos_count + 1,
                             sizeof(v3f))) {
          success = false;
          goto cleanup;
        }
        positions[pos_count++] = (v3f){x, y, z};
        if (!out->has_bounds) {
          out->bounds_min = out->bounds_max = (v3f){x, y, z};
          out->has_bounds = true;
        } else {
          if (x < out->bounds_min.x)
            out->bounds_min.x = x;
          if (y < out->bounds_min.y)
            out->bounds_min.y = y;
          if (z < out->bounds_min.z)
            out->bounds_min.z = z;
          if (x > out->bounds_max.x)
            out->bounds_max.x = x;
          if (y > out->bounds_max.y)
            out->bounds_max.y = y;
          if (z > out->bounds_max.z)
            out->bounds_max.z = z;
        }
      }
      continue;
    }
    if (line[0] == 'v' && line[1] == 't') {
      float u, v;
      if (sscanf(line + 2, "%f %f", &u, &v) == 2) {
        if (!ensure_capacity((void **)&uvs, &uv_cap, uv_count + 1,
                             sizeof(v2f))) {
          success = false;
          goto cleanup;
        }
        uvs[uv_count++] = (v2f){u, v};
      }
      continue;
    }
    if (line[0] == 'f' && isspace((unsigned char)line[1])) {
      char *cursor = line + 1;
      char *tokens[16];
      int tcount = 0;
      while (*cursor) {
        while (isspace((unsigned char)*cursor)) {
          cursor++;
        }
        if (*cursor == '\0' || *cursor == '\n') {
          break;
        }
        if (tcount >= (int)(sizeof(tokens) / sizeof(tokens[0]))) {
          break;
        }
        tokens[tcount++] = cursor;
        while (*cursor && !isspace((unsigned char)*cursor)) {
          cursor++;
        }
        if (*cursor) {
          *cursor++ = '\0';
        }
      }
      if (tcount < 3) {
        continue;
      }

      int base_vi[16], base_ti[16];
      for (int i = 0; i < tcount; i++) {
        base_vi[i] = -1;
        base_ti[i] = -1;
      }
      for (int i = 0; i < tcount; i++) {
        int vi, ti, ni;
        if (!parse_index_triplet(tokens[i], &vi, &ti, &ni)) {
          continue;
        }
        int pos_idx = (vi < 0) ? pos_count + vi : vi - 1;
        int uv_idx = (ti < 0) ? uv_count + ti : ti - 1;
        base_vi[i] = pos_idx;
        base_ti[i] = uv_idx;
      }

      for (int i = 1; i < tcount - 1; i++) {
        if (!ensure_capacity((void **)&faces, &face_cap, face_count + 1,
                             sizeof(ObjFace))) {
          success = false;
          goto cleanup;
        }
        ObjFace *face = &faces[face_count++];
        memset(face, 0, sizeof(*face));
        face->mat = current_mat;
        int idx0 = base_vi[0], idx1 = base_vi[i], idx2 = base_vi[i + 1];
        int uv0 = base_ti[0], uv1 = base_ti[i], uv2 = base_ti[i + 1];

        face->v[0].pos = (idx0 >= 0 && idx0 < pos_count) ? positions[idx0]
                                                         : (v3f){0};
        face->v[1].pos = (idx1 >= 0 && idx1 < pos_count) ? positions[idx1]
                                                         : (v3f){0};
        face->v[2].pos = (idx2 >= 0 && idx2 < pos_count) ? positions[idx2]
                                                         : (v3f){0};

        face->v[0].uv = (uv0 >= 0 && uv0 < uv_count) ? uvs[uv0] : (v2f){0};
        face->v[1].uv = (uv1 >= 0 && uv1 < uv_count) ? uvs[uv1] : (v2f){0};
        face->v[2].uv = (uv2 >= 0 && uv2 < uv_count) ? uvs[uv2] : (v2f){0};
      }
    }
  }

cleanup:
  fclose(f);

  if (!success) {
    free(positions);
    free(uvs);
    free(faces);
    obj_model_free(out);
    return false;
  }

  free(positions);
  free(uvs);

  out->faces = faces;
  out->face_count = face_count;
  return true;
}

void obj_model_free(ObjModel *model) {
  if (!model) {
    return;
  }
  if (model->faces) {
    free(model->faces);
    model->faces = NULL;
  }
  for (int i = 0; i < model->material_count; i++) {
    if (model->materials[i].has_diffuse) {
      texture_destroy(&model->materials[i].diffuse);
    }
  }
  if (model->materials) {
    free(model->materials);
    model->materials = NULL;
  }
  model->face_count = 0;
  model->material_count = 0;
  model->has_bounds = false;
}
