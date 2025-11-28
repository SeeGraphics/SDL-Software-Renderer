#include "math.h"
#include <math.h>

float v3_dot(v3f a, v3f b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

v3f v3_cross(v3f a, v3f b) {
  return (v3f){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
               a.x * b.y - a.y * b.x};
}

v3f v3_add(v3f a, v3f b) { return (v3f){a.x + b.x, a.y + b.y, a.z + b.z}; }

v3f v3_sub(v3f a, v3f b) { return (v3f){a.x - b.x, a.y - b.y, a.z - b.z}; }

v3f v3_scale(v3f a, float s) { return (v3f){a.x * s, a.y * s, a.z * s}; }

v3f v3_normalize(v3f a) {
  float len_sq = v3_dot(a, a);
  if (len_sq == 0.0f) {
    return (v3f){0};
  }
  float inv = 1.0f / sqrtf(len_sq);
  return v3_scale(a, inv);
}

mat4 mat4_identity(void) {
  mat4 m = {0};
  m.m[0][0] = m.m[1][1] = m.m[2][2] = m.m[3][3] = 1.0f;
  return m;
}

mat4 mat4_mul(mat4 a, mat4 b) {
  mat4 r = {0};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] +
                  a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
    }
  }
  return r;
}

v4f mat4_mul_v4(mat4 m, v4f v) {
  v4f r;
  r.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
  r.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
  r.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
  r.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;
  return r;
}

mat4 mat4_translate(v3f t) {
  mat4 m = mat4_identity();
  m.m[0][3] = t.x;
  m.m[1][3] = t.y;
  m.m[2][3] = t.z;
  return m;
}

mat4 mat4_perspective(float fov_radians, float aspect, float znear, float zfar) {
  float f = 1.0f / tanf(fov_radians * 0.5f);
  mat4 m = {0};
  m.m[0][0] = f / aspect;
  m.m[1][1] = f;
  m.m[2][2] = (zfar + znear) / (znear - zfar);
  m.m[2][3] = (2.0f * zfar * znear) / (znear - zfar);
  m.m[3][2] = -1.0f;
  return m;
}

mat4 mat4_look_at(v3f eye, v3f target, v3f up) {
  v3f zaxis = v3_normalize(v3_sub(eye, target)); // camera forward
  v3f xaxis = v3_normalize(v3_cross(up, zaxis));
  v3f yaxis = v3_cross(zaxis, xaxis);

  mat4 m = mat4_identity();
  m.m[0][0] = xaxis.x;
  m.m[0][1] = xaxis.y;
  m.m[0][2] = xaxis.z;
  m.m[0][3] = -v3_dot(xaxis, eye);

  m.m[1][0] = yaxis.x;
  m.m[1][1] = yaxis.y;
  m.m[1][2] = yaxis.z;
  m.m[1][3] = -v3_dot(yaxis, eye);

  m.m[2][0] = zaxis.x;
  m.m[2][1] = zaxis.y;
  m.m[2][2] = zaxis.z;
  m.m[2][3] = -v3_dot(zaxis, eye);

  return m;
}

mat4 mat4_rotate_x(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  mat4 m = mat4_identity();
  m.m[1][1] = c;
  m.m[1][2] = -s;
  m.m[2][1] = s;
  m.m[2][2] = c;
  return m;
}

mat4 mat4_rotate_y(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  mat4 m = mat4_identity();
  m.m[0][0] = c;
  m.m[0][2] = s;
  m.m[2][0] = -s;
  m.m[2][2] = c;
  return m;
}
