#pragma once

#include "types.h"

float v3_dot(v3f a, v3f b);
v3f v3_cross(v3f a, v3f b);
v3f v3_add(v3f a, v3f b);
v3f v3_sub(v3f a, v3f b);
v3f v3_scale(v3f a, float s);
v3f v3_normalize(v3f a);

mat4 mat4_identity(void);
mat4 mat4_mul(mat4 a, mat4 b);
v4f mat4_mul_v4(mat4 m, v4f v);
mat4 mat4_translate(v3f t);
mat4 mat4_perspective(float fov_radians, float aspect, float znear, float zfar);
mat4 mat4_look_at(v3f eye, v3f target, v3f up);
mat4 mat4_rotate_x(float angle);
mat4 mat4_rotate_y(float angle);
