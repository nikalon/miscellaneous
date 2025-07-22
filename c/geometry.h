// @TODO: Document conventions used throughout the library
#pragma once

typedef float  f32;

// ====================================================================================================================
// Types
typedef struct {
    f32 x;
    f32 y;
} Vec2;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Vec3;

typedef struct {
    f32 elems[16];
} Mat44;

// ====================================================================================================================
// Vec2
Vec2 operator-(Vec2 v);
Vec2 operator+(Vec2 v, Vec2 w);
Vec2 operator-(Vec2 v, Vec2 w);
Vec2 operator*(f32 k,  Vec2 v);
Vec2 operator*(Vec2 v, f32 k);
Vec2 operator/(Vec2 v, f32 k);

f32  vec2_length(Vec2 v);
Vec2 vec2_normalize(Vec2 v);
f32  vec2_dot(Vec2 v, Vec2 w);

// ====================================================================================================================
// Vec3
Vec3 operator-(Vec3 v);
Vec3 operator+(Vec3 v, Vec3 w);
Vec3 operator-(Vec3 v, Vec3 w);
Vec3 operator*(f32 k,  Vec3 v);
Vec3 operator*(Vec3 v, f32 k);
Vec3 operator/(Vec3 v, f32 k);

f32  vec3_length(Vec3 v);
Vec3 vec3_normalize(Vec3 v);
f32  vec3_dot(Vec3 v, Vec3 w);
Vec3 vec3_cross(Vec3 v, Vec3 w);

// ====================================================================================================================
// Mat44
Mat44 operator+(Mat44 a, Mat44 b);
Mat44 operator-(Mat44 a, Mat44 b);
