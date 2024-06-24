#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

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

#include <gccore.h>

#define NORMALIZE_ORTHO_X (1.0f / 320.0f)
#define NORMALIZE_ORTHO_Y (-1.0f / 200.0f)
#define NORMALIZE_ORTHO (1.0f / 100.0f)
#define NORMALIZE_COLOUR (1.0f / 256.0f)

#define ADJUST_RESOLUTION_Y ( 0.75f )
#define OFFSET_X (-0.5f)
#define OFFSET_Y (0.375f)

#define kMinZCull 0
struct Vec3 cameraOffset;
FixP_t walkingBias = 0;
FixP_t playerHeight = 0;

struct Texture *nativeTextures[TOTAL_TEXTURES];
extern struct Texture *itemSprites[TOTAL_ITEMS];

#define GEOMETRY_SCALE_X 2.0f
#define GEOMETRY_SCALE_Y 2.0f
#define GEOMETRY_SCALE_Z 2.0f
#define GEOMETRY_TEXTURE_SCALE_X 1.0f
#define GEOMETRY_TEXTURE_SCALE_Y 1.0f

#define BIAS (intToFix(8))
#define REVERSE_BIAS (1.0f/8.0f)
#define FOG_MAX_DISTANCE 32.0f
#define Z_NEAR_PLANE_FRUSTUM 0

extern GXRModeObj *rmode;
extern Mtx model, modelview;
extern guVector cubeAxis;
extern GXRModeObj *rmode;
extern Mtx model, modelview;
extern Mtx view;
extern Mtx44 perspective;
extern guVector Yaxis;
extern guVector Xaxis;

extern GXTexObj whiteTextureObj;

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
            (struct Texture *) allocMem(sizeof(struct Texture), BITMAP_MEMORY, TRUE);

    toReturn->raw = loadBitmap(filename);

    return toReturn;
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

    FixP_t acc;
    FixP_t scaled;

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
    vx1 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos1.mX) + centerX;
    vy1 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos1.mY) + centerY;
    vz1 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos1.mZ) + centerZ;

    u2 = 1.0f - (uv2.x) / 16.0f;
    v2 = 1.0f - ((uv2.y) / 16.0f);
    vx2 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos2.mX) + centerX;
    vy2 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos2.mY) + centerY;
    vz2 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos2.mZ) + centerZ;

    u3 = 1.0f - (uv3.x) / 16.0f;
    v3 = 1.0f - ((uv3.y) / 16.0f);
    vx3 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos3.mX) + centerX;
    vy3 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos3.mY) + centerY;
    vz3 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos3.mZ) + centerZ;

    u4 = 1.0f - (uv4.x) / 16.0f;
    v4 = 1.0f - ((uv4.y) / 16.0f);
    vx4 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos4.mX) + centerX;
    vy4 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos4.mY) + centerY;
    vz4 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos4.mZ) + centerZ;


    struct Bitmap* bitmap = texture->raw;

    if (bitmap->nativeBuffer == NULL || bitmap->uploadId == -1) {
        submitBitmapToGPU(bitmap);
    }

    bindTexture(bitmap);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

    GX_Position3f32(vx1, vy1, vz1);
    GX_Color3f32(1, 1, 1);
    GX_TexCoord2f32(u1, v1);

    GX_Position3f32(vx2, vy2, vz2);
    GX_Color3f32(1, 1, 1);
    GX_TexCoord2f32(u2, v2);

    GX_Position3f32(vx4, vy4, vz4);
    GX_Color3f32(1, 1, 1);
    GX_TexCoord2f32(u4, v4);

    GX_Position3f32(vx3, vy3, vz3);
    GX_Color3f32(1, 1, 1);
    GX_TexCoord2f32(u3, v3);

    GX_End();

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

        drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
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

    drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, bitmap, 1);
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
        drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
    }

    if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {
        p2.mX = p3.mX = p0.mX = p1.mX = intToFix(1);
        drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
    }

    p0.mX = p2.mX = -intToFix(1);
    p1.mX = p3.mX = intToFix(1);

    if ((mask & MASK_BEHIND)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = -intToFix(1);
        drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
    }

    if ((mask & MASK_FRONT)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = intToFix(1);
        drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, enableAlpha);
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

        drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
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

        drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
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

    drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
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

    drawQuad( center, p0, uv0, p1, uv1, p2, uv2, p3, uv3, texture, 0);
}

void fillTriangle(int *coords, FramebufferPixelFormat tint) {

    float r, g, b, x, y;

    r = (tint & 0xFF) * NORMALIZE_COLOUR;
    g = ((tint & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
    b = ((tint & 0xFF0000) >> 16) * NORMALIZE_COLOUR;

    GX_LoadTexObj(&whiteTextureObj, GX_TEXMAP0);

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);

    x = OFFSET_X + coords[0] * NORMALIZE_ORTHO_X;
    y = OFFSET_Y + coords[1] * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;

    GX_Position3f32(x, y, -0.125);
    GX_Color3f32(r, g, b);
    GX_TexCoord2f32(1, 1);

    x = OFFSET_X + coords[2] * NORMALIZE_ORTHO_X;
    y = OFFSET_Y + coords[3] * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;

    GX_Position3f32(x, y, -0.125);
    GX_Color3f32(r, g, b);
    GX_TexCoord2f32(1, 1);

    x = OFFSET_X + coords[4] * NORMALIZE_ORTHO_X;
    y = OFFSET_Y + coords[5] * NORMALIZE_ORTHO_Y * ADJUST_RESOLUTION_Y;

    GX_Position3f32(x, y, -0.125);
    GX_Color3f32(r, g, b);
    GX_TexCoord2f32(1, 1);

    GX_End();
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

    FixP_t acc;
    FixP_t scaled;

    float centerX;
    float centerY;
    float centerZ;

    float vx1, vy1, vz1, u1, v1;
    float vx2, vy2, vz2, u2, v2;
    float vx3, vy3, vz3, u3, v3;

    acc = center.mY + playerHeight + walkingBias + yCameraOffset;
    scaled = Mul(acc, BIAS);
    centerY = GEOMETRY_SCALE_Y * (fixToInt(scaled) * REVERSE_BIAS);
    centerX = GEOMETRY_SCALE_X * (fixToInt(Mul(center.mX, BIAS)) * 0.5f * REVERSE_BIAS);
    centerZ = -GEOMETRY_SCALE_Z * (fixToInt(Mul(center.mZ, BIAS)) * 0.5f * REVERSE_BIAS);

    u1 = 1.0f - (uv1.x) / 16.0f;
    v1 = 1.0f - ((uv1.y) / 16.0f);
    vx1 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos1.mX) + centerX;
    vy1 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos1.mY) + centerY;
    vz1 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos1.mZ) + centerZ;

    u2 = 1.0f - (uv2.x) / 16.0f;
    v2 = 1.0f - ((uv2.y) / 16.0f);
    vx2 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos2.mX) + centerX;
    vy2 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos2.mY) + centerY;
    vz2 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos2.mZ) + centerZ;

    u3 = 1.0f - (uv3.x) / 16.0f;
    v3 = 1.0f - ((uv3.y) / 16.0f);
    vx3 = GEOMETRY_SCALE_X * 0.5f * fixToFloat(pos3.mX) + centerX;
    vy3 = -GEOMETRY_SCALE_Y * 0.5f * fixToFloat(pos3.mY) + centerY;
    vz3 = GEOMETRY_SCALE_Z * 0.5f * fixToFloat(pos3.mZ) + centerZ;

    struct Bitmap* bitmap = texture->raw;

    if (bitmap->nativeBuffer == NULL || bitmap->uploadId == -1) {
        submitBitmapToGPU(bitmap);
    }

    bindTexture(bitmap);

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);

    GX_Position3f32(vx1, vy1, vz1);
    GX_Color3f32(1, 1, 1);
    GX_TexCoord2f32(u1, v1);

    GX_Position3f32(vx2, vy2, vz2);
    GX_Color3f32(1, 1, 1);
    GX_TexCoord2f32(u2, v2);

    GX_Position3f32(vx3, vy3, vz3);
    GX_Color3f32(1, 1, 1);
    GX_TexCoord2f32(u3, v3);

    GX_End();

}

void drawMesh(struct Mesh *mesh, const struct Vec3 center, enum EDirection rotation) {
    int c;
    int count = mesh->triangleCount;
    struct Vec3 origin;
    FixP_t *vertexData = mesh->geometry;
    uint8_t *uvData = mesh->uvCoords;

    origin.mX = origin.mY = origin.mZ = 0;

    if (/*mesh->texture != NULL && (center.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM*/ TRUE) {
        guVector up = {0.0F, 1.0F, 0.0F};
        guMtxIdentity(model);
        guMtxRotAxisDeg(model, &up, rotation * 90);
        guMtxTransApply(model, model, fixToFloat(center.mX + xCameraOffset), fixToFloat(center.mY), -fixToFloat(center.mZ + zCameraOffset));
        guMtxConcat(view, model, modelview);
        GX_LoadPosMtxImm(modelview, GX_PNMTX3);
        GX_SetCurrentMtx(GX_PNMTX3);

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

            drawTriangle( origin,p1, uv1, p2, uv2, p3, uv3, mesh->texture, 0);

            vertexData += 9;
        }

        guMtxIdentity(model);
        guMtxTransApply(model, model, 0.0f, 0.0f, -0.8f);
        guMtxConcat(view, model, modelview);
        GX_LoadPosMtxImm(modelview, GX_PNMTX3);
        GX_SetCurrentMtx(GX_PNMTX3);
    }
}
