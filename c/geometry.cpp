#include <math.h>
#include "geometry.h"

// ====================================================================================================================
// Vec2
Vec2 operator-(Vec2 v) {
    return Vec2 {
        -v.x,
        -v.y
    };
}

Vec2 operator+(Vec2 v, Vec2 w) {
    return Vec2 {
        v.x + w.x,
        v.y + w.y
    };
}

Vec2 operator-(Vec2 v, Vec2 w) {
    return Vec2 {
        v.x - w.x,
        v.y - w.y
    };
}

Vec2 operator*(f32 k, Vec2 v) {
    return Vec2 {
        k*v.x,
        k*v.y
    };
}

Vec2 operator*(Vec2 v, f32 k) {
    return Vec2 {
        k*v.x,
        k*v.y
    };
}

Vec2 operator/(Vec2 v, f32 k) {
    return Vec2 {
        v.x/k,
        v.y/k
    };
}

f32 vec2_length(Vec2 v) {
    f32 l = sqrtf(v.x*v.x + v.y*v.y);
    return l;
}

Vec2 vec2_normalize(Vec2 v) {
    f32 l = vec2_length(v);
    Vec2 ret = v/l;
    return ret;
}

f32 vec2_dot(Vec2 v, Vec2 w) {
    f32 ret = v.x*w.x + v.y*w.y;
    return ret;
}

// ====================================================================================================================
// Vec3
Vec3 operator-(Vec3 v) {
    return Vec3 {
        -v.x,
        -v.y,
        -v.z
    };
}

Vec3 operator+(Vec3 v, Vec3 w) {
    return Vec3 {
        v.x + w.x,
        v.y + w.y,
        v.z + w.z
    };
}

Vec3 operator-(Vec3 v, Vec3 w) {
    return Vec3 {
        v.x - w.x,
        v.y - w.y,
        v.z - w.z
    };
}

Vec3 operator*(f32 k, Vec3 v) {
    return Vec3 {
        k*v.x,
        k*v.y,
        k*v.z
    };
}

Vec3 operator*(Vec3 v, f32 k) {
    return Vec3 {
        k*v.x,
        k*v.y,
        k*v.z
    };
}

Vec3 operator/(Vec3 v, f32 k) {
    return Vec3 {
        v.x/k,
        v.y/k,
        v.z/k
    };
}

f32 vec3_length(Vec3 v) {
    f32 l = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return l;
}

Vec3 vec3_normalize(Vec3 v) {
    f32 l = vec3_length(v);
    Vec3 ret = v/l;
    return ret;
}

f32 vec3_dot(Vec3 v, Vec3 w) {
    f32 ret = v.x*w.x + v.y*w.y + v.z*w.z;
    return ret;
}

Vec3 vec3_cross(Vec3 v, Vec3 w) {
    return Vec3 {
        v.y*w.z - v.z*w.y,
        v.z*w.x - v.x*w.z,
        v.x*w.y - v.y*w.x,
    };
}

// ====================================================================================================================
// Mat44
Mat44 operator+(Mat44 a, Mat44 b) {
    Mat44 ret = {};
    ret.elems[ 0] = a.elems[ 0] + b.elems[ 0];
    ret.elems[ 1] = a.elems[ 1] + b.elems[ 1];
    ret.elems[ 2] = a.elems[ 2] + b.elems[ 2];
    ret.elems[ 3] = a.elems[ 3] + b.elems[ 3];
    ret.elems[ 4] = a.elems[ 4] + b.elems[ 4];
    ret.elems[ 5] = a.elems[ 5] + b.elems[ 5];
    ret.elems[ 6] = a.elems[ 6] + b.elems[ 6];
    ret.elems[ 7] = a.elems[ 7] + b.elems[ 7];
    ret.elems[ 8] = a.elems[ 8] + b.elems[ 8];
    ret.elems[ 9] = a.elems[ 9] + b.elems[ 9];
    ret.elems[10] = a.elems[10] + b.elems[10];
    ret.elems[11] = a.elems[11] + b.elems[11];
    ret.elems[12] = a.elems[12] + b.elems[12];
    ret.elems[13] = a.elems[13] + b.elems[13];
    ret.elems[14] = a.elems[14] + b.elems[14];
    ret.elems[15] = a.elems[15] + b.elems[15];

    return ret;
}

Mat44 operator-(Mat44 a, Mat44 b) {
    Mat44 ret = {};
    ret.elems[ 0] = a.elems[ 0] - b.elems[ 0];
    ret.elems[ 1] = a.elems[ 1] - b.elems[ 1];
    ret.elems[ 2] = a.elems[ 2] - b.elems[ 2];
    ret.elems[ 3] = a.elems[ 3] - b.elems[ 3];
    ret.elems[ 4] = a.elems[ 4] - b.elems[ 4];
    ret.elems[ 5] = a.elems[ 5] - b.elems[ 5];
    ret.elems[ 6] = a.elems[ 6] - b.elems[ 6];
    ret.elems[ 7] = a.elems[ 7] - b.elems[ 7];
    ret.elems[ 8] = a.elems[ 8] - b.elems[ 8];
    ret.elems[ 9] = a.elems[ 9] - b.elems[ 9];
    ret.elems[10] = a.elems[10] - b.elems[10];
    ret.elems[11] = a.elems[11] - b.elems[11];
    ret.elems[12] = a.elems[12] - b.elems[12];
    ret.elems[13] = a.elems[13] - b.elems[13];
    ret.elems[14] = a.elems[14] - b.elems[14];
    ret.elems[15] = a.elems[15] - b.elems[15];

    return ret;
}
