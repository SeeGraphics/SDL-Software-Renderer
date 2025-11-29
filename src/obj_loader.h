#pragma once

#include "types.h"
#include <stdbool.h>

typedef struct {
  char name[64];
  Texture diffuse;
  bool has_diffuse;
} ObjMaterial;

typedef struct {
  Vertex3D v[3];
  ObjMaterial *mat;
} ObjFace;

typedef struct {
  ObjFace *faces;
  int face_count;
  ObjMaterial *materials;
  int material_count;
  v3f bounds_min;
  v3f bounds_max;
  bool has_bounds;
} ObjModel;

bool obj_model_load(const char *obj_path, ObjModel *out);
void obj_model_free(ObjModel *model);
