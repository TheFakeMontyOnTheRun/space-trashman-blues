#include <math.h>
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
#include "CRenderer.h"
#include "Engine.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "Matrices.h"

#ifdef SDLGL
#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#include <SDL_opengl.h>
#else
#ifndef ANDROID
#if __APPLE__
#include <TargetConditionals.h>
#define GL_SILENCE_DEPRECATION
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS, tvOS, or watchOS Simulator
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif TARGET_OS_MACCATALYST
// Mac's Catalyst (ports iOS API into Mac, like UIKit).
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif TARGET_OS_IPHONE
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
// iOS, tvOS, or watchOS device
#elif TARGET_OS_MAC
// Other kinds of Apple platforms
#include <OpenGL/gl.h>
#else
#   error "Unknown Apple platform"
#endif
#endif
#else

#include <GLES2/gl2.h>

#endif
#endif

struct VBORegister submitVBO(float *vertexData, float *uvData, int vertices,
                             unsigned short *indexData,
                             unsigned int indices);

struct Vec3 cameraOffset;
FixP_t walkingBias = 0;
FixP_t playerHeight = 0;

struct Texture *nativeTextures[TOTAL_TEXTURES];
extern struct Texture *itemSprites[TOTAL_ITEMS];

#define GEOMETRY_SCALE_X 2.0f
#define GEOMETRY_SCALE_Y 1.0f
#define GEOMETRY_SCALE_Z 2.0f
#define GEOMETRY_TEXTURE_SCALE_X 1.0f
#define GEOMETRY_TEXTURE_SCALE_Y 1.0f

#define BIAS (intToFix(8))
#define REVERSE_BIAS (1.0f/8.0f)
#define FOG_MAX_DISTANCE 32.0f
#define Z_NEAR_PLANE_FRUSTUM 0

extern unsigned int aPositionAttributeLocation;
extern unsigned int aTexCoordAttributeLocation;
extern unsigned int uScaleUniformLocation;
extern unsigned int uProjectionMatrixUniformLocation;
extern unsigned int sTextureUniformLocation;
extern unsigned int uModUniformLocation;
extern unsigned int uFadeUniformLocation;

extern t_mat4x4 viewMatrix;
extern t_mat4x4 transformMatrix;
extern t_mat4x4 rotateXMatrix;
extern t_mat4x4 rotateYMatrix;
extern t_mat4x4 rotateZMatrix;

extern unsigned int uViewMatrixUniformLocation;
extern unsigned int uTransformMatrixUniformLocation;
extern unsigned int uRotateXMatrixUniformLocation;
extern unsigned int uRotateYMatrixUniformLocation;
extern unsigned int uRotateZMatrixUniformLocation;


extern struct VBORegister planeXYVBO, leftFarVBO, leftNearVBO, floorVBO, rampVBO, planeYZVBO;

#define NORMALIZE_COLOUR (1.0f / 256.0f)

float uvTemp[8];

void renderVBOAt(struct Bitmap *bitmap,
                 struct VBORegister vbo,
                 float x, float y, float z,
                 int16_t rx, int16_t ry, int16_t rz,
                 float scaleX, float scaleY,
                 float u0, float v0,
                 float u1, float v1,
                 uint32_t tint,
                 uint8_t repeatTextures) {

    checkGLError("starting to draw VBO");


    if (repeatTextures) {
        glUniform2f(uScaleUniformLocation, scaleX, scaleY);
    }

    checkGLError("Setting texture scale");

    if (tint != 0xFFFFFFFF) {
        float r = (tint & 0xFF) * NORMALIZE_COLOUR;
        float g = ((tint & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
        float b = ((tint & 0xFF0000) >> 16) * NORMALIZE_COLOUR;
        glUniform4f(uModUniformLocation, r, g, b, 1.0f);
    }

    float vanishingFade = 1.0f - ( -z / 64.0f );
    glUniform4f(uFadeUniformLocation, vanishingFade, vanishingFade, vanishingFade, vanishingFade);

    checkGLError("Setting tint");

    bindTexture(bitmap);

    checkGLError("Texture bound");

    glBindBuffer(GL_ARRAY_BUFFER, vbo.vertexDataIndex);


    checkGLError("vertex data bound");

    glVertexAttribPointer(aPositionAttributeLocation, 3, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 3, 0);

    checkGLError("vertex data configured");


    glBindBuffer(GL_ARRAY_BUFFER, vbo.uvDataIndex);

    checkGLError("vertex indices bound");

    if (!isnan(u0)) {
        uvTemp[0] = u0;
        uvTemp[1] = v0;
        uvTemp[2] = u1;
        uvTemp[3] = v0;
        uvTemp[4] = u1;
        uvTemp[5] = v1;
        uvTemp[6] = u0;
        uvTemp[7] = v1;

        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 2, &uvTemp[0], GL_DYNAMIC_DRAW);
        checkGLError("uv data provided");
    }

    glVertexAttribPointer(aTexCoordAttributeLocation, 2, GL_FLOAT, GL_TRUE,
                          sizeof(float) * 2, 0);


    checkGLError("uv data configured");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indicesIndex);

    checkGLError("indices elements bound");

    mat4x4_transform(transformMatrix,
                     x + fixToFloat(xCameraOffset),
                     y - fixToFloat(yCameraOffset) + fixToFloat(playerHeight),
                     z - fixToFloat(zCameraOffset), scaleX, scaleY, 1);

    glUniformMatrix4fv(uTransformMatrixUniformLocation, 1, GL_FALSE, transformMatrix);

    if (rx != 0) {
        mat4x4_rotateX(rotateXMatrix, rx);
        glUniformMatrix4fv(uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix);
    }


    if (ry != 0) {
        mat4x4_rotateY(rotateYMatrix, ry);
        glUniformMatrix4fv(uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix);
    }

    if (rz != 0) {
        mat4x4_rotateZ(rotateZMatrix, rz);
        glUniformMatrix4fv(uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix);
    }

    checkGLError("matrices set");

    glDrawElements(GL_TRIANGLES, vbo.indices, GL_UNSIGNED_SHORT, 0);

    checkGLError("triangles drawn");
    
    if (repeatTextures) {
        glUniform2f(uScaleUniformLocation, 1.0f, 1.0f);
    }

    if (tint != 0xFFFFFFFF) {
        glUniform4f(uModUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    if (rx != 0) {
        mat4x4_rotateX(rotateXMatrix, 0);
        glUniformMatrix4fv(uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix);
    }
    
    
    if (ry != 0) {
        mat4x4_rotateY(rotateYMatrix, 0);
        glUniformMatrix4fv(uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix);
    }
    
    if (rz != 0) {
        mat4x4_rotateZ(rotateZMatrix, 0);
        glUniformMatrix4fv(uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix);
    }


    checkGLError("disabling attributes");
}

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

#ifdef TILED_BITMAPS
    if (mapTopLevel[0]) {
        for (c = 0; c < 8; ++c) {
            releaseBitmap(mapTopLevel[c]);
            mapTopLevel[c] = NULL;
        }
    }
#else
    if (mapTopLevel) {
        releaseBitmap(mapTopLevel);
        mapTopLevel = NULL;
    }
#endif

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

    return toReturn;
}

void drawRampAt(const struct Vec3 center0, const struct Vec3 center1,
                const struct Texture *texture, uint8_t direction, uint8_t flipTexture) {

    if (/*(center0.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM && (center1.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM*/ TRUE) {

        struct Vec2i uv0, uv1, uv2, uv3;
        struct Vec3 p0, p1, p2, p3, center;

        FixP_t geometryScale = (center1.mY - center0.mY);
        float r = 0;
        switch (direction) {
            case kNorth: {
                r = 180;
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
                r = 0;
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
                r = 270;
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
                r = 90;
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

        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, rampVBO, x, y, z, 0, r, 0, 1.0f, fixToFloat(geometryScale), 0, 0,
                    1, 1, 0xFFFFFFFF, FALSE);
    }
}

void drawBillboardAt(const struct Vec3 center,
                     struct Texture *texture,
                     const FixP_t scale,
                     const int size) {

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


    float x = fixToFloat(center.mX);
    float y = fixToFloat(center.mY);
    float z = -fixToFloat(center.mZ);

    renderVBOAt(texture->raw, planeXYVBO, x, y, z, 0, leanX, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                0xFFFFFFFF, FALSE);
}

void drawColumnAt(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t enableAlpha,
                  const uint8_t repeatTexture) {
/*
    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }
*/
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

        float x = fixToFloat(center.mX) - 1.0f;
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, planeYZVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                    0xFFFFFFFF, repeatTexture);
    }

    if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {
        p2.mX = p3.mX = p0.mX = p1.mX = intToFix(1);

        float x = fixToFloat(center.mX) + 1.0f;
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, planeYZVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                    0xFFFFFFFF, repeatTexture);
    }

    p0.mX = p2.mX = -intToFix(1);
    p1.mX = p3.mX = intToFix(1);

    if ((mask & MASK_BEHIND)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = -intToFix(1);
        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ) - 1;
        renderVBOAt(texture->raw, planeXYVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                    0xFFFFFFFF, repeatTexture);
    }

    if ((mask & MASK_FRONT)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = intToFix(1);

        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ) + 1;
        renderVBOAt(texture->raw, planeXYVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                    0xFFFFFFFF, repeatTexture);
    }
}

void drawFloorAt(const struct Vec3 center,
                 const struct Texture *texture, enum EDirection cameraDirection) {

    if (/*center.mY <= 0 && (center.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM*/ TRUE) {
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

        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, floorVBO, x, y, z, 0, 0, 0, 1.0f, 1.0f, 0, 0, 1, 1, 0xFFFFFFFF,
                    FALSE);
    }
}

void drawCeilingAt(const struct Vec3 center,
                   const struct Texture *texture, enum EDirection cameraDirection) {

    if (/*center.mY >= 0 && center.mZ > Z_NEAR_PLANE_FRUSTUM*/ TRUE) {
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

        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, floorVBO, x, y, z, 0, 0, 0, 1.0f, 1.0f, 0, 0, 1, 1, 0xFFFFFFFF,
                    FALSE);
    }
}

void drawLeftNear(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t repeatTexture) {

/*
    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }
*/
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

    float x = fixToFloat(center.mX);
    float y = fixToFloat(center.mY);
    float z = -fixToFloat(center.mZ);

    if (cameraDirection == kWest || cameraDirection == kEast) {
        renderVBOAt(
                texture->raw, leftFarVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                0xFFFFFFFF, repeatTexture);
    } else {
        renderVBOAt(
                texture->raw, leftNearVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                0xFFFFFFFF, repeatTexture);
    }
}

void drawRightNear(const struct Vec3 center,
                   const FixP_t scale,
                   const struct Texture *texture,
                   const uint8_t mask,
                   const uint8_t repeatTexture) {
/*
    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }
*/
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

    float x = fixToFloat(center.mX);
    float y = fixToFloat(center.mY);
    float z = -fixToFloat(center.mZ);

    if (cameraDirection == kWest || cameraDirection == kEast) {
        renderVBOAt(texture->raw, leftNearVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1,
                    1, 0xFFFFFFFF, repeatTexture);
    } else {
        renderVBOAt(texture->raw, leftFarVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
                    0xFFFFFFFF, repeatTexture);
    }
}

void drawTriangle(const struct Vec3 pos1,
                  const struct Vec2i uv1,
                  const struct Vec3 pos2,
                  const struct Vec2i uv2,
                  const struct Vec3 pos3,
                  const struct Vec2i uv3,
                  const struct Texture *texture) {


}

void drawMesh(struct Mesh *mesh, const struct Vec3 center, enum EDirection rotation) {
    uint32_t c;
    uint32_t count = mesh->triangleCount;
    FixP_t *vertexData = mesh->geometry;
    uint8_t *uvData = mesh->uvCoords;

    if (mesh->nativeBuffer == NULL) {

        mesh->nativeVertexBuffer = calloc( 3 * 3 * count, sizeof(float));
        mesh->nativeTexCoordBuffer = calloc( 2 * 3 * count, sizeof(float));
        mesh->nativeIndicesBuffer = calloc( 3 * count, sizeof(unsigned short));

        float*  vP = mesh->nativeVertexBuffer;
        float* tP = mesh->nativeTexCoordBuffer;
        unsigned short* iP = mesh->nativeIndicesBuffer;

        for (c = 0; c < count; ++c) {

            float uv1x = 1.0f - ((*uvData++) / 16.0f);
            float uv1y = 1.0f - ((*uvData++) / 16.0f);
            float p1mX = fixToFloat(*(vertexData + 0));
            float p1mY = fixToFloat(*(vertexData + 1));
            float p1mZ = fixToFloat(*(vertexData + 2));

            float uv2x = 1.0f - ((*uvData++) / 16.0f);
            float uv2y = 1.0f - ((*uvData++) / 16.0f);
            float p2mX = fixToFloat(*(vertexData + 3));
            float p2mY = fixToFloat(*(vertexData + 4));
            float p2mZ = fixToFloat(*(vertexData + 5));

            float uv3x = 1.0f - ((*uvData++) / 16.0f);
            float uv3y = 1.0f - ((*uvData++) / 16.0f);
            float p3mX = fixToFloat(*(vertexData + 6));
            float p3mY = fixToFloat(*(vertexData + 7));
            float p3mZ = fixToFloat(*(vertexData + 8));

            *vP++ = p1mX;
            *vP++ = p1mY;
            *vP++ = p1mZ;

            *vP++ = p2mX;
            *vP++ = p2mY;
            *vP++ = p2mZ;

            *vP++ = p3mX;
            *vP++ = p3mY;
            *vP++ = p3mZ;

            *tP++ = uv1x;
            *tP++ = uv1y;

            *tP++ = uv2x;
            *tP++ = uv2y;

            *tP++ = uv3x;
            *tP++ = uv3y;

            *iP++ =  (c * 3) + 0;
            *iP++ =  (c * 3) + 1;
            *iP++ =  (c * 3) + 2;

            vertexData += 9;
        }

        mesh->nativeBuffer = calloc(1, sizeof(struct VBORegister));

        *((struct VBORegister*)mesh->nativeBuffer) = submitVBO((float *) mesh->nativeVertexBuffer, mesh->nativeTexCoordBuffer, count * 3,
                               (unsigned short *) mesh->nativeIndicesBuffer, count * 3);

    }

    {
        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);

        renderVBOAt( mesh->texture->raw, *((struct VBORegister*)mesh->nativeBuffer), x, y, z, 0, 360 - (rotation * 90), 0, 1.0f, 1.0f, NAN, 0, 1, 1,
                    0xFFFFFFFF, 0);

    }
}
