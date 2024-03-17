#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include <kernel.h>
#include <stdlib.h>
#include <malloc.h>
#include <tamtypes.h>
#include <math3d.h>
#include <packet.h>
#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>
#include <dma.h>
#include <graph.h>
#include <draw.h>
#include <draw3d.h>


#include "Core.h"
#include "Common.h"
#include "FixP.h"
#include "Enums.h"
#include "Vec.h"
#include "CActor.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "MapWithCharKey.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "LoadBitmap.h"
#include "Renderer.h"
#include "Engine.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"

#define NORMALIZE_ORTHO_X (1.0f / 320.0f)
#define NORMALIZE_ORTHO_Y (1.0f / 200.0f)
#define ADJUST_RESOLUTION_Y (((200.0f/256.0f) * 200.0f) / 240.0f )
#define OFFSET_X (-0.5f)
#define OFFSET_Y (-0.375f)
#define NORMALIZE_COLOUR (2.0f / 256.0f)

#define kMinZCull 0
struct Vec3 cameraOffset;
FixP_t walkingBias = 0;
FixP_t playerHeight = 0;

struct Texture *nativeTextures[TOTAL_TEXTURES];
extern struct Texture *itemSprites[TOTAL_ITEMS];

extern MATRIX local_world;
extern MATRIX world_view;
extern MATRIX view_screen;
extern MATRIX local_screen;
extern packet_t *current;

extern qword_t *_q;
extern xyz_t *verts;
extern texel_t *st;
extern color_t *colors;
extern VECTOR *temp_vertices;
extern int vertex_count;
extern prim_t prim;
extern color_t color;

#define GEOMETRY_SCALE_X 2.0f
#define GEOMETRY_SCALE_Y 1.0f
#define GEOMETRY_SCALE_Z 2.0f
#define GEOMETRY_TEXTURE_SCALE_X 1.0f
#define GEOMETRY_TEXTURE_SCALE_Y 1.0f

#define BIAS (intToFix(8))
#define REVERSE_BIAS (1.0f/8.0f)
#define FOG_MAX_DISTANCE 32.0f
#define Z_NEAR_PLANE_FRUSTUM 0

void clearTextures(void) {
    int c;
    texturesUsed = 0;
    for (c = 1; c < TOTAL_ITEMS; ++c) {
        if (itemSprites[c]) {
            releaseBitmap(itemSprites[c]->raw);
            disposeMem(itemSprites[c]);
            itemSprites[c] = NULL;
        }
    }

    if (mapTopLevel) {
        releaseBitmap(mapTopLevel);
        mapTopLevel = NULL;
    }

    for (c = 0; c < texturesUsed; ++c) {
        if (nativeTextures[c]) {
            releaseBitmap(nativeTextures[c]->raw);
            disposeMem(nativeTextures[c]);
            nativeTextures[c] = NULL;
        }
    }
}

float clampf(float v0, float v1, float v) {
    if (v < v0) {
        return v0;
    } else if (v > v1) {
        return v1;
    } else {
        return v;
    }
}


struct Texture *makeTextureFrom(const char *filename) {
    struct Texture *toReturn =
            (struct Texture *) calloc(1, sizeof(struct Texture));

    toReturn->raw = loadBitmap(filename);
    submitBitmapToGPU(toReturn->raw);

    return toReturn;
}

int isInsideFrustum(const struct Vec3 v, const struct Vec3 center) {
    FixP_t acc = center.mY + playerHeight + walkingBias + yCameraOffset;
    FixP_t scaled = Mul(acc, BIAS);
    float centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);
    float centerX = GEOMETRY_SCALE_X * (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
    float centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

    float x = centerX + GEOMETRY_SCALE_X * 0.5f * fixToFloat(v.mX);
    float y = centerY -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(v.mY);
    float z = centerZ + GEOMETRY_SCALE_Z * 0.5f * fixToFloat(v.mZ);

    float threshold = (-8.0f + (z * 2.0f ));

    if (x < threshold || x > -threshold) {
        return 0;
    }

    if (y < threshold || y > -threshold) {
        return 0;
    }

    if (z > 0.0f || z < -1024.0f) {
        return 0;
    }

    return 1;
}

void drawQuad(
        const struct Vec3 center,
        const struct Vec3 pos1,
        const struct Vec2i uv1,
        const struct Vec3 pos2,
        const struct Vec2i uv2,
        const struct Vec3 pos3,
        const struct Vec2i uv3,
        const struct Vec3 pos4,
        const struct Vec2i uv4,
        const struct Texture *texture,
        const uint8_t enableAlpha) {

    int allPointsInside = isInsideFrustum(pos1, center) &&
            isInsideFrustum(pos2, center) &&
            isInsideFrustum(pos3, center) &&
            isInsideFrustum(pos4, center);



    if (!allPointsInside) {
        /* Eventually, clipping will go here, for partially visible quads */
        return;
    } else {
        qword_t *q;
        u64 *dw;
        FixP_t acc;
        FixP_t scaled;

        int points_count = 6;

        int points[6] = {
                0, 1, 2,
                1, 2, 3
        };

        float centerX;
        float centerY;
        float centerZ;

        float vx1, vy1, vz1, u1, v1;
        float vx2, vy2, vz2, u2, v2;
        float vx3, vy3, vz3, u3, v3;
        float vx4, vy4, vz4, u4, v4;


        acc = center.mY + playerHeight + walkingBias + yCameraOffset;
        scaled = Mul(acc, BIAS);
        centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);
        centerX = GEOMETRY_SCALE_X * (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
        centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

        u1 = 1.0f - (uv1.x) / 16.0f;
        v1 = 1.0f - ((uv1.y) / 16.0f);
        vx1 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos1.mX);
        vy1 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos1.mY);
        vz1 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos1.mZ);

        u2 = 1.0f - (uv2.x) / 16.0f;
        v2 = 1.0f - ((uv2.y) / 16.0f);
        vx2 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos2.mX);
        vy2 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos2.mY);
        vz2 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos2.mZ);

        u3 = 1.0f - (uv3.x) / 16.0f;
        v3 = 1.0f - ((uv3.y) / 16.0f);
        vx3 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos3.mX);
        vy3 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos3.mY);
        vz3 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos3.mZ);

        u4 = 1.0f - (uv4.x) / 16.0f;
        v4 = 1.0f - ((uv4.y) / 16.0f);
        vx4 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos4.mX);
        vy4 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos4.mY);
        vz4 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos4.mZ);

        float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

        bindTexture(texture->raw);

        VECTOR colours[4] = {
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
        };

        VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
        VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

        create_local_world(local_world, object_position, object_rotation);

        create_local_screen(local_screen, local_world, world_view, view_screen);

        q = _q;

        VECTOR vertices[4] = {
                {vx1, vy1, vz1, 1.00f},
                {vx2, vy2, vz2, 1.00f},
                {vx3, vy3, vz3, 1.00f},
                {vx4, vy4, vz4, 1.00f}
        };

        // Calculate the vertex values.
        calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

        VECTOR coordinates[4] = {
                {u1, v1, 0, 0},
                {u2, v2, 0, 0},
                {u3, v3, 0, 0},
                {u4, v4, 0, 0}
        };

        draw_convert_st(st, vertex_count, (vertex_f_t *) temp_vertices, (texel_f_t *) coordinates);

        // Convert floating point vertices to fixed point and translate to center of screen.
        draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

        // Convert floating point colours to fixed point.
        draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

        // Draw the triangles using triangle primitive type.
        dw = (u64 *) draw_prim_start(q, 0, &prim, &color);

        for (int i = 0; i < points_count; i++) {
            *dw++ = colors[points[i]].rgbaq;
            *dw++ = st[points[i]].uv;
            *dw++ = verts[points[i]].xyz;
        }

        // Check if we're in middle of a qword or not.
        if ((u32) dw % 16) {
            *dw++ = 0;
        }

        q = draw_prim_end((qword_t *) dw, 3, DRAW_STQ_REGLIST);

        ++q;

        _q = q;
    }
}

void drawRampAt(const struct Vec3 center0, const struct Vec3 center1,
                const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {

    if ((center0.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM && (center1.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM) {

        struct Vec2i uv0, uv1, uv2, uv3;
        struct Vec3 p0, p1, p2, p3, center;

        FixP_t geometryScale = (center1.mY - center0.mY);

        switch (direction) {
            case kNorth: {
                uv0.x = 0;
                uv0.y = 0;
                uv1.x = 16;
                uv1.y = 0;
                uv2.x = 0;
                uv2.y = 16;
                uv3.x = 16;
                uv3.y = 16;

                p0.mX = p2.mX = -intToFix(1);
                p1.mX = p3.mX = intToFix(1);

                p0.mY = p1.mY = -geometryScale;
                p2.mY = p3.mY = geometryScale;

                p0.mZ = p1.mZ = -intToFix(1);
                p2.mZ = p3.mZ = intToFix(1);
            }
                break;
            case kSouth: {
                uv0.x = 16;
                uv0.y = 16;
                uv1.x = 0;
                uv1.y = 16;
                uv2.x = 16;
                uv2.y = 0;
                uv3.x = 0;
                uv3.y = 0;

                p0.mX = p2.mX = -intToFix(1);
                p1.mX = p3.mX = intToFix(1);

                p0.mY = p1.mY = geometryScale;
                p2.mY = p3.mY = -geometryScale;

                p0.mZ = p1.mZ = -intToFix(1);
                p2.mZ = p3.mZ = intToFix(1);
            }
                break;
            case kWest: {
                uv0.x = 0;
                uv0.y = 16;
                uv1.x = 0;
                uv1.y = 0;
                uv2.x = 16;
                uv2.y = 16;
                uv3.x = 16;
                uv3.y = 0;

                p0.mX = p2.mX = -intToFix(1);
                p1.mX = p3.mX = intToFix(1);

                p0.mY = p2.mY = geometryScale;
                p1.mY = p3.mY = -geometryScale;

                p0.mZ = p1.mZ = -intToFix(1);
                p2.mZ = p3.mZ = intToFix(1);
            }
                break;
            case kEast:
            default: {
                uv0.x = 16;
                uv0.y = 0;
                uv1.x = 16;
                uv1.y = 16;
                uv2.x = 0;
                uv2.y = 0;
                uv3.x = 0;
                uv3.y = 16;

                p0.mX = p2.mX = -intToFix(1);
                p1.mX = p3.mX = intToFix(1);

                p0.mY = p2.mY = -geometryScale;
                p1.mY = p3.mY = geometryScale;

                p0.mZ = p1.mZ = -intToFix(1);
                p2.mZ = p3.mZ = intToFix(1);
            }
                break;
        }

        center.mX = center0.mX;
        center.mY = Div(center0.mY + center1.mY, intToFix(2));
        center.mZ = center0.mZ;

        drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
    }
}

void drawBillboardAt(const struct Vec3 center,
                     struct Texture *bitmap,
                     const FixP_t scale,
                     const int size) {
    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }

    FixP_t geometryScale = Mul(scale, intToFix(2));
    float textureScale = 16;
    struct Vec2i uv0, uv1, uv2, uv3;
    struct Vec3 p0, p1, p2, p3;

    uv0.x = 16;
    uv0.y = 0;
    uv1.x = 0;
    uv1.y = 0;
    uv2.x = 16;
    uv2.y = textureScale;
    uv3.x = 0;
    uv3.y = textureScale;

    p0.mX = p2.mX = -intToFix(1);
    p1.mX = p3.mX = intToFix(1);

    p0.mZ = p1.mZ = p2.mZ = p3.mZ = 0;

    p0.mY = p1.mY = geometryScale;
    p2.mY = p3.mY = -geometryScale;

    drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, bitmap, 0);
}

void drawColumnAt(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t enableAlpha,
                  const uint8_t repeatTexture) {

    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }

    FixP_t geometryScale = Mul(scale, intToFix(2));
    float textureScale = 16;
    struct Vec2i uv0, uv1, uv2, uv3;
    struct Vec3 p0, p1, p2, p3;

    if (repeatTexture) {
        textureScale *= (fixToFloat(geometryScale)) * 0.5f;
    }

    uv0.x = 16;
    uv0.y = 0;
    uv1.x = 0;
    uv1.y = 0;
    uv2.x = 16;
    uv2.y = textureScale;
    uv3.x = 0;
    uv3.y = textureScale;

    p0.mZ = p2.mZ = intToFix(1);
    p1.mZ = p3.mZ = -intToFix(1);
    p0.mY = p1.mY = geometryScale;
    p2.mY = p3.mY = -geometryScale;

    if (((mask & MASK_RIGHT) && fixToInt(center.mX) > 0) || (mask & MASK_FORCE_RIGHT)) {
        p2.mX = p3.mX = p0.mX = p1.mX = -intToFix(1);
        drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
    }

    if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {
        p2.mX = p3.mX = p0.mX = p1.mX = intToFix(1);
        drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
    }

    p0.mX = p2.mX = -intToFix(1);
    p1.mX = p3.mX = intToFix(1);

    if ((mask & MASK_BEHIND)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = -intToFix(1);
        drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
    }

    if ((mask & MASK_FRONT)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = intToFix(1);
        drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
    }
}

void drawFloorAt(const struct Vec3 center,
                 const struct Texture *texture, enum EDirection cameraDirection) {

    if (center.mY <= 0 && (center.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM) {
        struct Vec2i uv0, uv1, uv2, uv3;
        struct Vec3 p0, p1, p2, p3;

        switch (cameraDirection) {
            case kNorth: {
                uv0.x = 0;
                uv0.y = 0;
                uv1.x = 16;
                uv1.y = 0;
                uv2.x = 0;
                uv2.y = 16;
                uv3.x = 16;
                uv3.y = 16;
            }
                break;
            case kSouth: {
                uv0.x = 16;
                uv0.y = 16;
                uv1.x = 0;
                uv1.y = 16;
                uv2.x = 16;
                uv2.y = 0;
                uv3.x = 0;
                uv3.y = 0;
            }
                break;

            case kWest: {
                uv0.x = 0;
                uv0.y = 16;
                uv1.x = 0;
                uv1.y = 0;
                uv2.x = 16;
                uv2.y = 16;
                uv3.x = 16;
                uv3.y = 0;
            }
                break;
            case kEast:
            default: {
                uv0.x = 16;
                uv0.y = 0;
                uv1.x = 16;
                uv1.y = 16;
                uv2.x = 0;
                uv2.y = 0;
                uv3.x = 0;
                uv3.y = 16;
            }
                break;
        }

        p0.mX = p2.mX = -intToFix(1);
        p1.mX = p3.mX = intToFix(1);

        p0.mZ = p1.mZ = -intToFix(1);
        p2.mZ = p3.mZ = intToFix(1);

        p0.mY = p1.mY = p2.mY = p3.mY = 0;

        drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
    }
}

void drawCeilingAt(const struct Vec3 center,
                   const struct Texture *texture, enum EDirection cameraDirection) {

    if (center.mY >= 0 && center.mZ > Z_NEAR_PLANE_FRUSTUM) {
        struct Vec2i uv0, uv1, uv2, uv3;
        struct Vec3 p0, p1, p2, p3;

        switch (cameraDirection) {
            case kNorth: {
                uv0.x = 0;
                uv0.y = 0;
                uv1.x = 16;
                uv1.y = 0;
                uv2.x = 0;
                uv2.y = 16;
                uv3.x = 16;
                uv3.y = 16;
            }
                break;
            case kSouth: {
                uv0.x = 16;
                uv0.y = 16;
                uv1.x = 0;
                uv1.y = 16;
                uv2.x = 16;
                uv2.y = 0;
                uv3.x = 0;
                uv3.y = 0;
            }
                break;

            case kWest: {
                uv0.x = 0;
                uv0.y = 16;
                uv1.x = 0;
                uv1.y = 0;
                uv2.x = 16;
                uv2.y = 16;
                uv3.x = 16;
                uv3.y = 0;
            }
                break;
            case kEast:
            default: {
                uv0.x = 16;
                uv0.y = 0;
                uv1.x = 16;
                uv1.y = 16;
                uv2.x = 0;
                uv2.y = 0;
                uv3.x = 0;
                uv3.y = 16;
            }
                break;
        }

        p0.mX = p2.mX = -intToFix(1);
        p1.mX = p3.mX = intToFix(1);

        p0.mZ = p1.mZ = -intToFix(1);
        p2.mZ = p3.mZ = intToFix(1);

        p0.mY = p1.mY = p2.mY = p3.mY = 0;

        drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
    }
}

void drawLeftNear(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t repeatTexture) {


    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }

    FixP_t geometryScale = Mul(scale, intToFix(2));
    float textureScale = 16;
    struct Vec2i uv0, uv1, uv2, uv3;
    struct Vec3 p0, p1, p2, p3;

    if (repeatTexture) {
        textureScale *= (fixToFloat(geometryScale)) * 0.5f;
    }

    uv0.x = 16;
    uv0.y = 0;
    uv1.x = 0;
    uv1.y = 0;
    uv2.x = 16;
    uv2.y = textureScale;
    uv3.x = 0;
    uv3.y = textureScale;

    p0.mX = p2.mX = intToFix(1);
    p1.mX = p3.mX = -intToFix(1);

    if (cameraDirection == kWest || cameraDirection == kEast) {
        p0.mZ = p2.mZ = intToFix(1);
        p1.mZ = p3.mZ = -intToFix(1);
    } else {
        p0.mZ = p2.mZ = -intToFix(1);
        p1.mZ = p3.mZ = intToFix(1);
    }

    p0.mY = p1.mY = geometryScale;
    p2.mY = p3.mY = -geometryScale;

    drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
}

void drawRightNear(const struct Vec3 center,
                   const FixP_t scale,
                   const struct Texture *texture,
                   const uint8_t mask,
                   const uint8_t repeatTexture) {

    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }

    FixP_t geometryScale = Mul(scale, intToFix(2));
    float textureScale = 16;
    struct Vec2i uv0, uv1, uv2, uv3;
    struct Vec3 p0, p1, p2, p3;

    if (repeatTexture) {
        textureScale *= (fixToFloat(geometryScale)) * 0.5f;
    }

    uv0.x = 16;
    uv0.y = 0;
    uv1.x = 0;
    uv1.y = 0;
    uv2.x = 16;
    uv2.y = textureScale;
    uv3.x = 0;
    uv3.y = textureScale;

    p0.mX = p2.mX = intToFix(1);
    p1.mX = p3.mX = -intToFix(1);

    if (cameraDirection == kWest || cameraDirection == kEast) {
        p0.mZ = p2.mZ = -intToFix(1);
        p1.mZ = p3.mZ = intToFix(1);
    } else {
        p0.mZ = p2.mZ = intToFix(1);
        p1.mZ = p3.mZ = -intToFix(1);
    }

    p0.mY = p1.mY = geometryScale;
    p2.mY = p3.mY = -geometryScale;

    drawQuad(center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
}

void drawTriangle(
        const struct Vec3 center,
        const struct Vec3 pos1,
        const struct Vec2i uv1,
        const struct Vec3 pos2,
        const struct Vec2i uv2,
        const struct Vec3 pos3,
        const struct Vec2i uv3,
        const struct Texture *texture,
        const uint8_t enableAlpha) {

    int allPointsInside = isInsideFrustum(pos1, center) &&
                          isInsideFrustum(pos2, center) &&
                          isInsideFrustum(pos3, center);


    if (!allPointsInside) {
        /* Eventually, clipping will go here, for partially visible quads */
        return;
    } else {
        qword_t *q;
        u64 *dw;

        int points_count = 6;

        int points[3] = {
                0, 1, 2
        };

        float vx1, vy1, vz1, u1, v1;
        float vx2, vy2, vz2, u2, v2;
        float vx3, vy3, vz3, u3, v3;

        u1 = 1.0f - (uv1.x) / 16.0f;
        v1 = 1.0f - ((uv1.y) / 16.0f);
        vx1 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos1.mX);
        vy1 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos1.mY);
        vz1 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos1.mZ);

        u2 = 1.0f - (uv2.x) / 16.0f;
        v2 = 1.0f - ((uv2.y) / 16.0f);
        vx2 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos2.mX);
        vy2 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos2.mY);
        vz2 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos2.mZ);

        u3 = 1.0f - (uv3.x) / 16.0f;
        v3 = 1.0f - ((uv3.y) / 16.0f);
        vx3 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos3.mX);
        vy3 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos3.mY);
        vz3 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos3.mZ);
        float centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
        float fogAttenuation = 1.0f - (1.0f - (centerZ / FOG_MAX_DISTANCE));

        bindTexture(texture->raw);

        VECTOR colours[3] = {
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
                {fogAttenuation, fogAttenuation, fogAttenuation, 1.00f},
        };


        q = _q;

        VECTOR vertices[3] = {
                {vx1, vy1, vz1, 1.00f},
                {vx2, vy2, vz2, 1.00f},
                {vx3, vy3, vz3, 1.00f}
        };

        // Calculate the vertex values.
        calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

        VECTOR coordinates[3] = {
                {u1, v1, 0, 0},
                {u2, v2, 0, 0},
                {u3, v3, 0, 0}
        };

        draw_convert_st(st, vertex_count, (vertex_f_t *) temp_vertices, (texel_f_t *) coordinates);

        // Convert floating point vertices to fixed point and translate to center of screen.
        draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

        // Convert floating point colours to fixed point.
        draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

        // Draw the triangles using triangle primitive type.
        dw = (u64 *) draw_prim_start(q, 0, &prim, &color);

        for (int i = 0; i < points_count; i++) {
            *dw++ = colors[points[i]].rgbaq;
            *dw++ = st[points[i]].uv;
            *dw++ = verts[points[i]].xyz;
        }

        // Check if we're in middle of a qword or not.
        if ((u32) dw % 16) {
            *dw++ = 0;
        }

        q = draw_prim_end((qword_t *) dw, 3, DRAW_STQ_REGLIST);

        ++q;

        _q = q;
    }
}

void fillTriangle(int *coords, FramebufferPixelFormat pixel) {


    float r, g, b, a;

    r = (pixel & 0xFF) * NORMALIZE_COLOUR;
    g = ((pixel & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
    b = ((pixel & 0xFF0000) >> 16) * NORMALIZE_COLOUR;
    a = 1.0f;

    if (defaultFont == NULL) {
        defaultFont = loadBitmap("font.img");
    }

    if (defaultFont->uploadId == -1) {
        submitBitmapToGPU(defaultFont);
    }

    bindTexture(defaultFont);

    qword_t *q;

    int points_count = 6;

    int points[3] = {
            0, 1, 2
    };

    u64 *dw;
/*
    float x = OFFSET_X + _x * NORMALIZE_ORTHO_X;
    float y = OFFSET_Y + _y * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;
    float dx = _dx * NORMALIZE_ORTHO_X;
    float dy = _dy * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;
*/
    VECTOR object_position = {0, 0, -1.0f, 1.0f};
    VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

    create_local_world(local_world, object_position, object_rotation);

    create_local_screen(local_screen, local_world, world_view, view_screen);

    q = _q;

    VECTOR vertices[3] = {
            {OFFSET_X + coords[0] * NORMALIZE_ORTHO_X, OFFSET_Y + coords[1] * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y, 0, 1.00f},
            {OFFSET_X + coords[2] * NORMALIZE_ORTHO_X, OFFSET_Y + coords[3] * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y, 0, 1.00f},
            {OFFSET_X + coords[4] * NORMALIZE_ORTHO_X, OFFSET_Y + coords[5] * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y,  0, 1.00f},
    };

    VECTOR coordinates[3] = {
            {1, 0, 0, 0},
            {0, 0, 0, 0},
            {1, 1, 0, 0},
    };

    VECTOR colours[3] = {
            {r, g, b, a},
            {r, g, b, a},
            {r, g, b, a},
    };

    // Calculate the vertex values.
    calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

    draw_convert_st(st, vertex_count, (vertex_f_t *) temp_vertices, (texel_f_t *) coordinates);

    // Convert floating point vertices to fixed point and translate to center of screen.
    draw_convert_xyz(verts, 2048, 2048, 2048, vertex_count, (vertex_f_t *) temp_vertices);

    // Convert floating point colours to fixed point.
    draw_convert_rgbaq(colors, vertex_count, (vertex_f_t *) temp_vertices, (color_f_t *) colours);

    prim_t trig;

    trig.type = PRIM_TRIANGLE;
    trig.shading = PRIM_SHADE_GOURAUD;
    trig.mapping = DRAW_DISABLE;
    trig.fogging = DRAW_DISABLE;
    trig.blending = DRAW_ENABLE;
    trig.antialiasing = DRAW_DISABLE;
    trig.mapping_type = PRIM_MAP_ST;
    trig.colorfix = PRIM_UNFIXED;

    dw = (u64 *) draw_prim_start(q, 0, &trig, &color);

    for (int i = 0; i < points_count; i++) {
        *dw++ = colors[points[i]].rgbaq;
        *dw++ = st[points[i]].uv;
        *dw++ = verts[points[i]].xyz;
    }

    // Check if we're in middle of a qword or not.
    if ((u32) dw % 16) {
        *dw++ = 0;
    }

    q = draw_prim_end((qword_t *) dw, 3, DRAW_STQ_REGLIST);

    ++q;

    _q = q;
}

void drawMesh(struct Mesh *mesh, const struct Vec3 center, enum EDirection rotation) {
    int c;
    int count = mesh->triangleCount;
    FixP_t *vertexData = mesh->geometry;
    uint8_t *uvData = mesh->uvCoords;
    FixP_t acc;
    FixP_t scaled;

    float centerX;
    float centerY;
    float centerZ;

    acc = center.mY + playerHeight + walkingBias + yCameraOffset;
    scaled = Mul(acc, BIAS);
    centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);
    centerX = GEOMETRY_SCALE_X * (fixToInt(Mul(center.mX + xCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);
    centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ + zCameraOffset, BIAS)) * 0.5f * REVERSE_BIAS);

    VECTOR object_position = {centerX, centerY, centerZ, 1.00f};
    VECTOR object_rotation = {0.00f, (rotation * 90.00f) * (3.14159f / 180.0f), 0.00f, 1.00f};

    create_local_world(local_world, object_position, object_rotation);

    create_local_screen(local_screen, local_world, world_view, view_screen);

    if (mesh->texture != NULL && (center.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM) {
        for (c = 0; c < count; ++c) {
            struct Vec3 p1;
            struct Vec3 p2;
            struct Vec3 p3;
            struct Vec2i uv1;
            struct Vec2i uv2;
            struct Vec2i uv3;

            uv1.x = (*uvData++);
            uv1.y = (*uvData++);
            p1.mX = *(vertexData + 0);
            p1.mY = *(vertexData + 1);
            p1.mZ = *(vertexData + 2);

            uv2.x = (*uvData++);
            uv2.y = (*uvData++);
            p2.mX = *(vertexData + 3);
            p2.mY = *(vertexData + 4);
            p2.mZ = *(vertexData + 5);

            uv3.x = (*uvData++);
            uv3.y = (*uvData++);
            p3.mX = *(vertexData + 6);
            p3.mY = *(vertexData + 7);
            p3.mZ = *(vertexData + 8);

            drawTriangle( center,p1, uv1, p2, uv2, p3, uv3, mesh->texture, 0);

            vertexData += 9;
        }
    }
}
