#include <math.h>
#include <stdio.h>
#include "Matrices.h"

void mat4x4_ortho(t_mat4x4 out, float left, float right, float bottom, float top, float znear,
                  float zfar) {
#define T(a, b) (a * 4 + b)

    out[T(0, 0)] = 2.0f / (right - left);
    out[T(0, 1)] = 0.0f;
    out[T(0, 2)] = 0.0f;
    out[T(0, 3)] = 0.0f;

    out[T(1, 1)] = 2.0f / (top - bottom);
    out[T(1, 0)] = 0.0f;
    out[T(1, 2)] = 0.0f;
    out[T(1, 3)] = 0.0f;

    out[T(2, 2)] = -2.0f / (zfar - znear);
    out[T(2, 0)] = 0.0f;
    out[T(2, 1)] = 0.0f;
    out[T(2, 3)] = 0.0f;

    out[T(3, 0)] = -(right + left) / (right - left);
    out[T(3, 1)] = -(top + bottom) / (top - bottom);
    out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
    out[T(3, 3)] = 1.0f;

#undef T
}


void mat4x4_perspective(t_mat4x4 out, float fov, float ratio, float znear, float zfar) {
    float rad = M_PI / 180.0f;
    float oneOverTanFovDiv2 = 1.0f / tan(fov * rad / 2.0f);
    out[0] = oneOverTanFovDiv2 / (ratio);
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;

    out[4] = 0.0f;
    out[5] = oneOverTanFovDiv2;
    out[6] = 0.0f;
    out[7] = 0.0f;

    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = -((zfar + znear) / (zfar - znear));
    out[11] = -1.0f;

    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = -((2.0f * zfar * znear) / (zfar - znear));
    out[15] = 0.0f;
}

void mat4x4_view(t_mat4x4 out, float cx, float cy, float cz, float tx, float ty, float tz, float ux,
                 float uy, float uz) {
    out[0] = 1;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;
    out[4] = 0.0f;
    out[5] = 1;
    out[6] = 0.0f;
    out[7] = 0.0f;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = 1;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}

void mat4x4_transform(t_mat4x4 out, float ox, float oy, float oz, float sx, float sy, float sz) {
    out[0] = sx;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0;
    out[4] = 0.0f;
    out[5] = sy;
    out[6] = 0.0f;
    out[7] = 0;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = sz;
    out[11] = 0;
    out[12] = ox;
    out[13] = oy;
    out[14] = oz;
    out[15] = 1;
}

void mat4x4_rotateX(t_mat4x4 out, float deg) {

    float ca = cosf(deg * M_PI / 180.0f);
    float sa = sinf(deg * M_PI / 180.0f);

    out[0] = 1;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;
    out[4] = 0.0f;
    out[5] = ca;
    out[6] = -sa;
    out[7] = 0.0f;
    out[8] = 0;
    out[9] = sa;
    out[10] = ca;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}

void mat4x4_rotateY(t_mat4x4 out, float deg) {

    float ca = cosf(deg * M_PI / 180.0f);
    float sa = sinf(deg * M_PI / 180.0f);

    out[0] = ca;
    out[1] = 0.0f;
    out[2] = sa;
    out[3] = 0.0f;
    out[4] = 0.0f;
    out[5] = 1;
    out[6] = 0.0f;
    out[7] = 0.0f;
    out[8] = -sa;
    out[9] = 0.0f;
    out[10] = ca;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}

void mat4x4_rotateZ(t_mat4x4 out, float deg) {

    float ca = cosf(deg * M_PI / 180.0f);
    float sa = sinf(deg * M_PI / 180.0f);
    out[0] = ca;
    out[1] = -sa;
    out[2] = 0.0f;
    out[3] = 0.0f;
    out[4] = sa;
    out[5] = ca;
    out[6] = 0.0f;
    out[7] = 0.0f;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = 1;
    out[11] = 0.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0;
    out[15] = 1;
}

void mat4x4_transpose( t_mat4x4 mat) {
    float tmp[16];
    /*
    a b c d
    e f g h
    i j k l
    m n o p
    
    
    a e i m
    b f j n
    c g k p
    d h l q
     */
    for ( int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x ) {
            tmp[y * 4 + x] = mat[x * 4 + y];
        }
    }
    
    for (int c = 0; c < 16; ++c ) {
        mat[c] = tmp[c];
    }
}