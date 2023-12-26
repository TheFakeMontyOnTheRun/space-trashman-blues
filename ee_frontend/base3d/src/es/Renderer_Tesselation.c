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

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

#define kMinZCull 0
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
extern unsigned int uModelPositionUniformLocation;


extern t_mat4x4 transformMatrix;
extern t_mat4x4 rotateXMatrix;
extern t_mat4x4 rotateYMatrix;
extern t_mat4x4 rotateZMatrix;

extern unsigned int uTransformMatrixUniformLocation;
extern unsigned int uRotateXMatrixUniformLocation;
extern unsigned int uRotateYMatrixUniformLocation;
extern unsigned int uRotateZMatrixUniformLocation;


extern struct VBORegister planeXYVBO, leftFarVBO, leftNearVBO, floorVBO, planeYZVBO;


void renderVBOAt( struct Bitmap* bitmap, struct VBORegister vbo, float x, float y, float z, float scaleX, float scaleY ) {
  glEnableVertexAttribArray(aPositionAttributeLocation);
  glEnableVertexAttribArray(aTexCoordAttributeLocation);
  
  glUniform4f(uModelPositionUniformLocation, x, y, z, 1.0f);

  glUniform2f(uScaleUniformLocation,scaleX, scaleY);


    bindTexture(bitmap);
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo.dataIndex);

    glVertexAttribPointer(aPositionAttributeLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
    glVertexAttribPointer(aTexCoordAttributeLocation, 2, GL_FLOAT, GL_TRUE,
                          sizeof(float) * 5, (void *) (sizeof(float) * 3));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indicesIndex);

    mat4x4_transform(transformMatrix, 0, 0, 0, 1, 1, 1);
    glUniformMatrix4fv( uTransformMatrixUniformLocation, 1, GL_FALSE, transformMatrix );

    mat4x4_rotateX(rotateXMatrix, leanY);
    glUniformMatrix4fv( uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix );

    mat4x4_rotateY(rotateYMatrix, leanX);
    glUniformMatrix4fv( uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix );

    mat4x4_rotateZ(rotateZMatrix, 0);
    glUniformMatrix4fv( uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix );


    glDrawElements(GL_TRIANGLES, vbo.indices, GL_UNSIGNED_SHORT, 0);


    mat4x4_transform(transformMatrix, 0, 0, 0, 1, 1, 1);
    glUniformMatrix4fv( uTransformMatrixUniformLocation, 1, GL_FALSE, transformMatrix );

    mat4x4_rotateX(rotateXMatrix, leanY);
    glUniformMatrix4fv( uRotateXMatrixUniformLocation, 1, GL_FALSE, rotateXMatrix );

    mat4x4_rotateY(rotateYMatrix, leanX);
    glUniformMatrix4fv( uRotateYMatrixUniformLocation, 1, GL_FALSE, rotateYMatrix );

    mat4x4_rotateZ(rotateZMatrix, 0);
    glUniformMatrix4fv( uRotateZMatrixUniformLocation, 1, GL_FALSE, rotateZMatrix );


    glDisableVertexAttribArray(aPositionAttributeLocation);
  glDisableVertexAttribArray(aTexCoordAttributeLocation);
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


    }
}

void drawBillboardAt(const struct Vec3 center,
                     struct Texture *texture,
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


    float x = fixToFloat(center.mX);
    float y = fixToFloat(center.mY);
    float z = -fixToFloat(center.mZ);
    
    renderVBOAt(texture->raw, planeXYVBO, x, y, z, 1.0f, fixToFloat(scale));
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

        float x = fixToFloat(center.mX) - 1.0f;
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, planeYZVBO, x, y, z, 1.0f, fixToFloat(scale));
    }

    if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {
        p2.mX = p3.mX = p0.mX = p1.mX = intToFix(1);

        float x = fixToFloat(center.mX) + 1.0f;
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, planeYZVBO, x, y, z, 1.0f, fixToFloat(scale));
    }

    p0.mX = p2.mX = -intToFix(1);
    p1.mX = p3.mX = intToFix(1);

    if ((mask & MASK_BEHIND)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = -intToFix(1);
        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ) - 1;
        renderVBOAt(texture->raw, planeXYVBO, x, y, z, 1.0f, fixToFloat(scale));
    }

    if ((mask & MASK_FRONT)) {
        p2.mZ = p3.mZ = p0.mZ = p1.mZ = intToFix(1);

        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ) + 1;
        renderVBOAt(texture->raw, planeXYVBO, x, y, z, 1.0f, fixToFloat(scale));
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

        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, floorVBO, x, y, z, 1.0f, 1.0f);
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

        float x = fixToFloat(center.mX);
        float y = fixToFloat(center.mY);
        float z = -fixToFloat(center.mZ);
        renderVBOAt(texture->raw, floorVBO, x, y, z, 1.0f, 1.0f);
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

    float x = fixToFloat(center.mX);
    float y = fixToFloat(center.mY);
    float z = -fixToFloat(center.mZ);

    if (cameraDirection == kWest || cameraDirection == kEast) {
        renderVBOAt(
                texture->raw, leftFarVBO, x, y, z, 1.0f, fixToFloat(scale));
    } else {
        renderVBOAt(
                texture->raw, leftNearVBO, x, y, z, 1.0f, fixToFloat(scale));
    }
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

    float x = fixToFloat(center.mX);
    float y = fixToFloat(center.mY);
    float z = -fixToFloat(center.mZ);

    if (cameraDirection == kWest || cameraDirection == kEast) {
        renderVBOAt(texture->raw, leftNearVBO, x, y, z, 1.0f, fixToFloat(scale));
    } else {
        renderVBOAt(texture->raw, leftFarVBO, x, y, z, 1.0f, fixToFloat(scale));
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
