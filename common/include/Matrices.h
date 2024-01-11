#ifndef MATRICES_H
#define MATRICES_H

typedef float t_mat4x4[16];

void mat4x4_ortho(t_mat4x4 out, float left, float right, float bottom, float top, float znear,
                  float zfar);

void mat4x4_view(t_mat4x4 out, float cx, float cy, float cz, float tx, float ty, float tz, float ux,
                 float uy, float uz);

void mat4x4_perspective(t_mat4x4 out, float fov, float ratio, float znear, float zfar);

void mat4x4_transform( t_mat4x4 out, float ox, float oy, float oz, float sx, float sy, float sz );

void mat4x4_rotateX( t_mat4x4 out, float deg );

void mat4x4_rotateY( t_mat4x4 out, float deg );

void mat4x4_rotateZ( t_mat4x4 out, float deg );

void mat4x4_transpose( t_mat4x4 mat);

#endif /* MATRICES_H */
