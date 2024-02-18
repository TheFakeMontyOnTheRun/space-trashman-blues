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

struct Vec3 cameraOffset;
FixP_t walkingBias = 0;
FixP_t playerHeight = 0;
extern struct Bitmap whiteTexture;
extern const float planeXYVertices[12];
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

void renderVBOAt(struct Bitmap *bitmap, struct VBORegister vbo, float x, float y, float z, float rx,
                 float ry, float rz, float scaleX, float scaleY, float u0, float v0, float u1,
                 float v1, uint32_t tint, uint8_t repeatTextures) {


    checkGLError("starting to draw VBO");
    glEnableVertexAttribArray(aPositionAttributeLocation);
    checkGLError("Enabled vertex position attribute");


    glEnableVertexAttribArray(aTexCoordAttributeLocation);
    checkGLError("Enabled vertex uv attribute");

    if (repeatTextures) {
        glUniform2f(uScaleUniformLocation, scaleX, scaleY);
    } else {
        glUniform2f(uScaleUniformLocation, 1.0f, 1.0f);
    }

    checkGLError("Setting texture scale");

    float r = (tint & 0xFF) * NORMALIZE_COLOUR;
    float g = ((tint & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
    float b = ((tint & 0xFF0000) >> 16) * NORMALIZE_COLOUR;

    glUniform4f(uModUniformLocation, r, g, b, 1.0f);

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

    glVertexAttribPointer(aTexCoordAttributeLocation, 2, GL_FLOAT, GL_TRUE,
                          sizeof(float) * 2, 0);


    checkGLError("uv data configured");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indicesIndex);


    checkGLError("indices elements bound");

    mat4x4_transform(transformMatrix, x + fixToFloat(xCameraOffset), y - fixToFloat(yCameraOffset),
                     z - fixToFloat(zCameraOffset), scaleX, scaleY, 1);
    glUniformMatrix4fv(uTransformMatrixUniformLocation, 1, GL_FALSE, transformMatrix);

    mat4x4_rotateX(rotateXMatrix, rx);
    glUniformMatrix4fv(uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix);

    mat4x4_rotateY(rotateYMatrix, ry);
    glUniformMatrix4fv(uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix);

    mat4x4_rotateZ(rotateZMatrix, rz);
    glUniformMatrix4fv(uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix);


    checkGLError("matrices set");


    glDrawElements(GL_TRIANGLES, vbo.indices, GL_UNSIGNED_SHORT, 0);


    checkGLError("triangles drawn");

    mat4x4_transform(transformMatrix, 0, 0, 0, 1, 1, 1);
    glUniformMatrix4fv(uTransformMatrixUniformLocation, 1, GL_FALSE, transformMatrix);

    mat4x4_rotateX(rotateXMatrix, leanY);
    glUniformMatrix4fv(uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix);

    mat4x4_rotateY(rotateYMatrix, leanX);
    glUniformMatrix4fv(uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix);

    mat4x4_rotateZ(rotateZMatrix, 0);
    glUniformMatrix4fv(uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix);


    checkGLError("unsetting matrices");

    glDisableVertexAttribArray(aPositionAttributeLocation);
    glDisableVertexAttribArray(aTexCoordAttributeLocation);

    checkGLError("disabling attributes");
}

void fillTriangle(int *coords, FramebufferPixelFormat fragment) {


    checkGLError("starting to draw VBO");
    glEnableVertexAttribArray(aPositionAttributeLocation);
    checkGLError("Enabled vertex position attribute");


    glEnableVertexAttribArray(aTexCoordAttributeLocation);
    checkGLError("Enabled vertex uv attribute");

    glUniform2f(uScaleUniformLocation, 1.0f, 1.0f);

    checkGLError("Setting texture scale");

    float r = (fragment & 0xFF) * NORMALIZE_COLOUR;
    float g = ((fragment & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
    float b = ((fragment & 0xFF0000) >> 16) * NORMALIZE_COLOUR;

    glUniform4f(uModUniformLocation, r, g, b, 1.0f);

    checkGLError("Setting tint");

    bindTexture(&whiteTexture);


    checkGLError("Texture bound");

    glBindBuffer(GL_ARRAY_BUFFER, planeXYVBO.vertexDataIndex);

    float trigVertex[12];
    trigVertex[0] = coords[0];
    trigVertex[1] = coords[1];
    trigVertex[2] = -1;
    trigVertex[3] = coords[2];
    trigVertex[4] = coords[3];
    trigVertex[5] = -1;
    trigVertex[6] = coords[4];
    trigVertex[7] = coords[5];
    trigVertex[8] = -1;
    trigVertex[9] = coords[4];
    trigVertex[10] = coords[5];
    trigVertex[11] = -1;

    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 3, &trigVertex[0], GL_DYNAMIC_DRAW);



    checkGLError("vertex data bound");

    glVertexAttribPointer(aPositionAttributeLocation, 3, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 3, 0);


    checkGLError("vertex data configured");


    glBindBuffer(GL_ARRAY_BUFFER, planeXYVBO.uvDataIndex);


    checkGLError("vertex indices bound");

    float uvTemp[8];
    uvTemp[0] = 1;
    uvTemp[1] = 1;
    uvTemp[2] = 1;
    uvTemp[3] = 1;
    uvTemp[4] = 1;
    uvTemp[5] = 1;
    uvTemp[6] = 1;
    uvTemp[7] = 1;

    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 2, &uvTemp[0], GL_DYNAMIC_DRAW);


    checkGLError("uv data provided");

    glVertexAttribPointer(aTexCoordAttributeLocation, 2, GL_FLOAT, GL_TRUE,
                          sizeof(float) * 2, 0);


    checkGLError("uv data configured");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeXYVBO.indicesIndex);


    checkGLError("indices elements bound");

    mat4x4_transform(transformMatrix, 0 + fixToFloat(xCameraOffset), 0 - fixToFloat(yCameraOffset),
                     - 1 - fixToFloat(zCameraOffset), 1, 1, 1);
    glUniformMatrix4fv(uTransformMatrixUniformLocation, 1, GL_FALSE, transformMatrix);

    mat4x4_rotateX(rotateXMatrix, 0);
    glUniformMatrix4fv(uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix);

    mat4x4_rotateY(rotateYMatrix, 0);
    glUniformMatrix4fv(uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix);

    mat4x4_rotateZ(rotateZMatrix, 0);
    glUniformMatrix4fv(uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix);


    checkGLError("matrices set");


    glDrawElements(GL_TRIANGLES, planeXYVBO.indices, GL_UNSIGNED_SHORT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, planeXYVBO.vertexDataIndex);

    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 3, &planeXYVertices[0], GL_DYNAMIC_DRAW);

    checkGLError("triangles drawn");

    mat4x4_transform(transformMatrix, 0, 0, 0, 1, 1, 1);
    glUniformMatrix4fv(uTransformMatrixUniformLocation, 1, GL_FALSE, transformMatrix);

    mat4x4_rotateX(rotateXMatrix, leanY);
    glUniformMatrix4fv(uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix);

    mat4x4_rotateY(rotateYMatrix, leanX);
    glUniformMatrix4fv(uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix);

    mat4x4_rotateZ(rotateZMatrix, 0);
    glUniformMatrix4fv(uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix);


    checkGLError("unsetting matrices");

    glDisableVertexAttribArray(aPositionAttributeLocation);
    glDisableVertexAttribArray(aTexCoordAttributeLocation);

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
    /*
    if ((center.mZ + zCameraOffset) <= Z_NEAR_PLANE_FRUSTUM) {
        return;
    }
     */

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

    renderVBOAt(texture->raw, planeXYVBO, x, y, z, 0, 0, 0, 1.0f, fixToFloat(scale), 0, 0, 1, 1,
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

void drawMesh(const struct Mesh *mesh, const struct Vec3 center) {
    int c;
    int count = mesh->triangleCount;
    FixP_t *vertexData = mesh->geometry;
    uint8_t *uvData = mesh->uvCoords;

    if (/*mesh->texture != NULL && (center.mZ + zCameraOffset) > Z_NEAR_PLANE_FRUSTUM*/ TRUE) {
        for (c = 0; c < count; ++c) {
            struct Vec3 p1;
            struct Vec3 p2;
            struct Vec3 p3;
            struct Vec2i uv1;
            struct Vec2i uv2;
            struct Vec2i uv3;

            uv1.x = (*uvData++);
            uv1.y = (*uvData++);
            p1.mX = center.mX + *(vertexData + 0);
            p1.mY = center.mY + *(vertexData + 1);
            p1.mZ = center.mZ + *(vertexData + 2);

            uv2.x = (*uvData++);
            uv2.y = (*uvData++);
            p2.mX = center.mX + *(vertexData + 3);
            p2.mY = center.mY + *(vertexData + 4);
            p2.mZ = center.mZ + *(vertexData + 5);

            uv3.x = (*uvData++);
            uv3.y = (*uvData++);
            p3.mX = center.mX + *(vertexData + 6);
            p3.mY = center.mY + *(vertexData + 7);
            p3.mZ = center.mZ + *(vertexData + 8);

            drawTriangle(p1, uv1, p2, uv2, p3, uv3, mesh->texture);

            vertexData += 9;
        }
    }
}
