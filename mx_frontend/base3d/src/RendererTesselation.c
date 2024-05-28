#include <string.h>
#include <stdlib.h>

#include <assert.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>
#include <math.h>

#endif

#include "Common.h"
#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Vec.h"
#include "Globals.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "Renderer.h"

#define kMinZCull 0
#define FIXP_HALF_XRES  (intToFix(HALF_XRES))
#define FIXP_HALF_YRES  (intToFix(HALF_YRES))
#define FIXP_ONE (intToFix(1))

FixP_t oneOverZMap[4 * 128];
FixP_t divLut[320];

#ifdef AGS
__attribute__((section(".iwram"), long_call))
#endif

void projectAllVertices(const uint8_t count) {

    FixP_t oneOver = FIXP_ONE;
    int c;

    struct Projection *vertex = &projectionVertices[0];
    FixP_t lastZ = 0xCAFEBABE;
    FixP_t compoundYFactor = playerHeight + walkingBias + yCameraOffset;

    for (c = 0; c < count; ++c, ++vertex) {

        FixP_t z = (vertex->first.mZ);

        if (z != lastZ) {
            lastZ = z;

            z += zCameraOffset;

            if (z < FIXP_ONE) {
                z = FIXP_ONE;
            }

            oneOver = oneOverZMap[fixToInt(z * 4)];
        }

        vertex->second.mX = (FIXP_HALF_XRES + Mul(vertex->first.mX + xCameraOffset, oneOver));
        vertex->second.mY = (FIXP_HALF_YRES - Mul(vertex->first.mY + compoundYFactor, oneOver));
    }
}


void initZMap(void) {
    int z;
    for (z = 1; z < (4 * 128); ++z) {
        oneOverZMap[z] = Div(FIXP_HALF_YRES, Div(intToFix(z), intToFix(4)));
    }

    for (z = 1; z < (320); ++z) {
        divLut[z] = Div(intToFix(1), intToFix(z));
    }
}


void drawBillboardAt(const struct Vec3 center,
                     struct Bitmap *bitmap,
                     const FixP_t scale,
                     const int size) {

    struct Vec2 ulz0;
    struct Vec2 lrz0;
    struct Vec3 *ptr0;
    struct Vec3 *ptr1;

    if (center.mZ <= kMinZCull) {
        return;
    }

    ptr0 = &projectionVertices[0].first;
    ptr1 = &projectionVertices[1].first;
    ptr0->mX = center.mX - FIXP_ONE;
    ptr1->mX = center.mX + FIXP_ONE;
    ptr0->mY = center.mY + scale;
    ptr1->mY = center.mY - scale;
    ptr0->mZ = ptr1->mZ = center.mZ;

    projectAllVertices(2);

    ulz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;

    if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
        drawMask(ulz0.mX, ulz0.mY, lrz0.mX, lrz0.mY);
    } else {
        drawFrontWall(ulz0.mX, ulz0.mY, lrz0.mX, lrz0.mY, bitmap->data,
                      scale, fixToInt(center.mZ), TRUE, size);
    }
}

void drawColumnAt(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t enableAlpha,
                  const uint8_t repeatedTexture) {

    const FixP_t halfScale = scale;
    FixP_t textureScale;
    int z;
    struct Vec2 p0;
    struct Vec2 p1;
    struct Vec2 p2;
    struct Vec2 p3;
    int originalZ;
    struct Vec3 *ptr0;
    struct Vec3 *ptr1;
    struct Vec3 *ptr2;
    struct Vec3 *ptr3;

    if (center.mZ <= kMinZCull) {
        return;
    }

    /*
         2|\             /|
          | \  center   / |
          |  \    *    /  |
          |   \0__|___/   |3
          |   |   |  |   /
           \  |   X  |  /
            \ |      | /
             \|_____1|/


            behind
             ___
     left    |_|  right

           front
  */
    textureScale = (repeatedTexture ? halfScale : FIXP_ONE);
    z = fixToInt(center.mZ);
    originalZ = z;

    ptr0 = &projectionVertices[0].first;
    ptr1 = &projectionVertices[1].first;
    ptr2 = &projectionVertices[2].first;
    ptr3 = &projectionVertices[3].first;

    ptr0->mX = ptr2->mX = center.mX - FIXP_ONE;
    ptr1->mX = ptr3->mX = center.mX + FIXP_ONE;

    ptr0->mY = ptr2->mY = center.mY + halfScale;
    ptr1->mY = ptr3->mY = center.mY - halfScale;

    ptr0->mZ = ptr1->mZ = center.mZ - FIXP_ONE;
    ptr2->mZ = ptr3->mZ = center.mZ + FIXP_ONE;

    projectAllVertices(4);

    p0 = projectionVertices[0].second;
    p1 = projectionVertices[1].second;
    p2 = projectionVertices[2].second;
    p3 = projectionVertices[3].second;

    if ((mask & MASK_BEHIND) || (enableAlpha && (mask & MASK_FRONT))) {
        if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
            drawMask(p2.mX, p2.mY, p3.mX, p3.mY);
        } else {
            drawFrontWall(p2.mX, p2.mY, p3.mX, p3.mY, texture->rotations[0],
                          (textureScale), z, enableAlpha, NATIVE_TEXTURE_SIZE);
        }
    }

    if (((mask & MASK_RIGHT) && fixToInt(center.mX) > 0) || (mask & MASK_FORCE_RIGHT)) {

        if (mask & ~MASK_BEHIND) {
            z -= 2;
        }

        if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
            maskWall(p2.mX, p0.mX, p2.mY, p3.mY, p0.mY, p1.mY);
        } else {
            drawWall(p2.mX, p0.mX, p2.mY, p3.mY, p0.mY, p1.mY, texture->rowMajor,
                     (textureScale), z);
        }

        z = originalZ;
    }

    if (((mask & MASK_LEFT) && fixToInt(center.mX) < 0) || (mask & MASK_FORCE_LEFT)) {
        if (mask & ~MASK_BEHIND) {
            z -= 2;
        }

        if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
            maskWall(p1.mX, p3.mX, p0.mY, p1.mY, p2.mY, p3.mY);
        } else {
            drawWall(p1.mX, p3.mX, p0.mY, p1.mY, p2.mY, p3.mY, texture->rowMajor,
                     (textureScale), z);
        }

        z = originalZ;
    }

    if ((mask & MASK_FRONT)) {
        if (mask & ~MASK_BEHIND) {
            z -= 2;
        }

        if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
            drawMask(p0.mX, p0.mY, p1.mX, p1.mY);
        } else {
            drawFrontWall(p0.mX, p0.mY, p1.mX, p1.mY, texture->rotations[0],
                          (textureScale), z, enableAlpha, NATIVE_TEXTURE_SIZE);
        }
    }
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
                const struct Texture *texture, uint8_t cameraDirection, uint8_t flipTexture) {

    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec2 llz1;
    struct Vec2 lrz1;
    uint8_t uvCoords[6];
    int coords[6];

    if (min(p0.mZ, p1.mZ) <= kMinZCull) {
        return;
    }

    if (cameraDirection == kNorth) {

        projectionVertices[0].first = projectionVertices[1].first = p0;
        projectionVertices[2].first = projectionVertices[3].first = p1;

        if (flipTexture) {
            cameraDirection = kSouth;
        }

    } else if (cameraDirection == kSouth) {

        projectionVertices[0].first = projectionVertices[1].first = p1;
        projectionVertices[2].first = projectionVertices[3].first = p0;

        if (flipTexture) {
            cameraDirection = kNorth;
        }

    } else {
        if (cameraDirection == kEast) {
            projectionVertices[0].first = projectionVertices[2].first = p1;
            projectionVertices[1].first = projectionVertices[3].first = p0;
        } else {
            projectionVertices[0].first = projectionVertices[2].first = p0;
            projectionVertices[1].first = projectionVertices[3].first = p1;
        }

        addToVec3(&projectionVertices[0].first, -FIXP_ONE, 0, -FIXP_ONE);
        addToVec3(&projectionVertices[1].first, FIXP_ONE, 0, -FIXP_ONE);
        addToVec3(&projectionVertices[2].first, -FIXP_ONE, 0, FIXP_ONE);
        addToVec3(&projectionVertices[3].first, FIXP_ONE, 0, FIXP_ONE);

        projectAllVertices(4);

        llz0 = projectionVertices[0].second;
        lrz0 = projectionVertices[1].second;
        llz1 = projectionVertices[2].second;
        lrz1 = projectionVertices[3].second;

        coords[0] = fixToInt(llz1.mX); /* 2 */
        coords[1] = fixToInt(llz1.mY);
        coords[2] = fixToInt(lrz1.mX); /* 3 */
        coords[3] = fixToInt(lrz1.mY);
        coords[4] = fixToInt(llz0.mX); /* 0 */
        coords[5] = fixToInt(llz0.mY);

        if (p0.mZ < FIXP_DISTANCE_FOR_DARKNESS) {
            if (flipTexture) {
                uvCoords[0] = 0;
                uvCoords[1] = NATIVE_TEXTURE_SIZE;
                uvCoords[2] = 0;
                uvCoords[3] = 0;
                uvCoords[4] = NATIVE_TEXTURE_SIZE;
                uvCoords[5] = NATIVE_TEXTURE_SIZE;
            } else {
                uvCoords[0] = NATIVE_TEXTURE_SIZE;
                uvCoords[1] = 0;
                uvCoords[2] = NATIVE_TEXTURE_SIZE;
                uvCoords[3] = NATIVE_TEXTURE_SIZE;
                uvCoords[4] = 0;
                uvCoords[5] = 0;
            }

            drawTexturedTriangle(&coords[0], &uvCoords[0], (struct Texture *) texture, fixToInt(p0.mZ));
        } else {
            fillTriangle(&coords[0], 0);
        }

        coords[0] = fixToInt(llz0.mX); /* 0 */
        coords[1] = fixToInt(llz0.mY);
        coords[2] = fixToInt(lrz1.mX); /* 3 */
        coords[3] = fixToInt(lrz1.mY);
        coords[4] = fixToInt(lrz0.mX); /* 1 */
        coords[5] = fixToInt(lrz0.mY);

        if (p0.mZ < FIXP_DISTANCE_FOR_DARKNESS) {
            if (flipTexture) {
                uvCoords[0] = NATIVE_TEXTURE_SIZE;
                uvCoords[1] = NATIVE_TEXTURE_SIZE;
                uvCoords[2] = 0;
                uvCoords[3] = 0;
                uvCoords[4] = NATIVE_TEXTURE_SIZE;
                uvCoords[5] = 0;
            } else {
                uvCoords[0] = 0;
                uvCoords[1] = 0;
                uvCoords[2] = NATIVE_TEXTURE_SIZE;
                uvCoords[3] = NATIVE_TEXTURE_SIZE;
                uvCoords[4] = 0;
                uvCoords[5] = NATIVE_TEXTURE_SIZE;
            }

            drawTexturedTriangle(&coords[0], &uvCoords[0], (struct Texture *) texture, fixToInt(p0.mZ));
        } else {
            fillTriangle(&coords[0], 0);
        }

        return;
    }

    addToVec3(&projectionVertices[0].first, -FIXP_ONE, 0, -FIXP_ONE);
    addToVec3(&projectionVertices[1].first, FIXP_ONE, 0, -FIXP_ONE);
    addToVec3(&projectionVertices[2].first, -FIXP_ONE, 0, FIXP_ONE);
    addToVec3(&projectionVertices[3].first, FIXP_ONE, 0, FIXP_ONE);

    projectAllVertices(4);

    llz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;
    llz1 = projectionVertices[2].second;
    lrz1 = projectionVertices[3].second;

    if (p0.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
        maskFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, 0);
    } else {
#ifndef FLOOR_TEXTURES_DONT_ROTATE
        drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, fixToInt(p0.mZ),
                  texture->rotations[cameraDirection]);
#else
        drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, fixToInt(p0.mZ), texture->rotations[0] );
#endif
    }
}

void drawFloorAt(const struct Vec3 center,
                 const struct Texture *texture, enum EDirection rotation) {

    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec2 llz1;
    struct Vec2 lrz1;
    struct Vec3 *ptr0;
    struct Vec3 *ptr1;
    struct Vec3 *ptr2;
    struct Vec3 *ptr3;

    if (center.mZ <= kMinZCull) {
        return;
    }

    ptr0 = &projectionVertices[0].first;
    ptr1 = &projectionVertices[1].first;
    ptr2 = &projectionVertices[2].first;
    ptr3 = &projectionVertices[3].first;

    ptr0->mX = ptr2->mX = center.mX - FIXP_ONE;
    ptr1->mX = ptr3->mX = center.mX + FIXP_ONE;

    ptr0->mY = ptr1->mY = ptr2->mY = ptr3->mY = center.mY;

    ptr0->mZ = ptr1->mZ = center.mZ - FIXP_ONE;
    ptr2->mZ = ptr3->mZ = center.mZ + FIXP_ONE;

    projectAllVertices(4);

    llz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;
    llz1 = projectionVertices[2].second;
    lrz1 = projectionVertices[3].second;

    if (center.mY <= 0) {
        if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
            maskFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, 0);
        } else {
#ifndef FLOOR_TEXTURES_DONT_ROTATE
            drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, fixToInt(center.mZ),
                      texture->rotations[cameraDirection]);
#else
            drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, fixToInt(center.mZ),
                  texture->rotations[0]);
#endif
        }
    }
}

void drawCeilingAt(const struct Vec3 center,
                   const struct Texture *texture, enum EDirection rotation) {
    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec2 llz1;
    struct Vec2 lrz1;
    struct Vec3 *ptr0;
    struct Vec3 *ptr1;
    struct Vec3 *ptr2;
    struct Vec3 *ptr3;

    if (center.mZ <= kMinZCull) {
        return;
    }

    ptr0 = &projectionVertices[0].first;
    ptr1 = &projectionVertices[1].first;
    ptr2 = &projectionVertices[2].first;
    ptr3 = &projectionVertices[3].first;

    ptr0->mX = ptr2->mX = center.mX - FIXP_ONE;
    ptr1->mX = ptr3->mX = center.mX + FIXP_ONE;

    ptr0->mY = ptr1->mY = ptr2->mY = ptr3->mY = center.mY;

    ptr0->mZ = ptr1->mZ = center.mZ - FIXP_ONE;
    ptr2->mZ = ptr3->mZ = center.mZ + FIXP_ONE;

    projectAllVertices(4);

    llz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;
    llz1 = projectionVertices[2].second;
    lrz1 = projectionVertices[3].second;

    if (center.mY >= 0) {

        if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
            maskFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, 0);
        } else {
#ifndef FLOOR_TEXTURES_DONT_ROTATE
            drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, fixToInt(center.mZ),
                      texture->rotations[cameraDirection]);
#else
            drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, fixToInt(center.mZ),
                      texture->rotations[0]);
#endif
        }
    }
}

void drawLeftNear(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *texture,
                  const uint8_t mask,
                  const uint8_t repeatedTexture) {

    FixP_t halfScale = scale;
    const FixP_t textureScale = (repeatedTexture ? halfScale : FIXP_ONE);
    FixP_t depth = FIXP_ONE;
    struct Vec2 ulz0;
    struct Vec2 urz0;
    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec3 *ptr0;
    struct Vec3 *ptr1;
    struct Vec3 *ptr2;
    struct Vec3 *ptr3;

    if (center.mZ <= kMinZCull) {
        return;
    }

    if (mask & MASK_BEHIND) {

        ptr0 = &projectionVertices[0].first;
        ptr1 = &projectionVertices[1].first;

        ptr0->mX = center.mX - FIXP_ONE;
        ptr1->mX = center.mX + FIXP_ONE;

        ptr0->mY = center.mY - halfScale;
        ptr1->mY = center.mY + halfScale;

        ptr0->mZ = center.mZ - FIXP_ONE;
        ptr1->mZ = center.mZ - FIXP_ONE;

        projectAllVertices(2);

        drawMask(projectionVertices[0].second.mX, projectionVertices[0].second.mY,
                 projectionVertices[1].second.mX, projectionVertices[1].second.mY);

        return;
    }

    if (cameraDirection == kWest || cameraDirection == kEast) {
        depth = -FIXP_ONE;
    }
    ptr0 = &projectionVertices[0].first;
    ptr1 = &projectionVertices[1].first;
    ptr2 = &projectionVertices[2].first;
    ptr3 = &projectionVertices[3].first;

    ptr0->mX = ptr2->mX = center.mX - FIXP_ONE;
    ptr1->mX = ptr3->mX = center.mX + FIXP_ONE;

    ptr0->mY = ptr1->mY = center.mY + halfScale;
    ptr2->mY = ptr3->mY = center.mY - halfScale;

    ptr0->mZ = ptr2->mZ = center.mZ - depth;
    ptr1->mZ = ptr3->mZ = center.mZ + depth;

    projectAllVertices(4);

    ulz0 = projectionVertices[0].second;
    urz0 = projectionVertices[1].second;
    llz0 = projectionVertices[2].second;
    lrz0 = projectionVertices[3].second;

    if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
        maskWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY);
    } else {
        drawWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY, texture->rowMajor,
                 textureScale, fixToInt(center.mZ));
    }
}

void drawMesh(const struct Mesh *mesh, const struct Vec3 center, enum EDirection rotation) {

    int coords[6];
    int count = mesh->triangleCount;
    FixP_t *vertexData = mesh->geometry;
    uint8_t colour = mesh->colour;
	struct Vec3 *ptr0;
    struct Vec3 *ptr1;
	struct Vec3 *ptr2;
    int c;	
    
    if (mesh->texture == NULL || center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
        for (c = 0; c < count; ++c) {
            float vx, vy, vz;

            vx = fixToFloat(*(vertexData + 0));
            vy = fixToFloat(*(vertexData + 3));
            vz = fixToFloat(*(vertexData + 6));

            ptr0 = &projectionVertices[0].first;
            ptr1 = &projectionVertices[1].first;
            ptr2 = &projectionVertices[2].first;

            switch (rotation) {
                case kEast:
                    ptr0->mX = center.mX - *(vertexData + 2);
                    ptr0->mZ = center.mZ + *(vertexData + 0);

                    ptr1->mX = center.mX - *(vertexData + 5);
                    ptr1->mZ = center.mZ + *(vertexData + 3);

                    ptr2->mX = center.mX - *(vertexData + 8);
                    ptr2->mZ = center.mZ + *(vertexData + 6);
                    break;
                case kWest:
                    ptr0->mX = center.mX + *(vertexData + 2);
                    ptr0->mZ = center.mZ - *(vertexData + 0);

                    ptr1->mX = center.mX + *(vertexData + 5);
                    ptr1->mZ = center.mZ - *(vertexData + 3);

                    ptr2->mX = center.mX + *(vertexData + 8);
                    ptr2->mZ = center.mZ - *(vertexData + 6);
                    break;
                case kSouth:
                    ptr0->mX = center.mX - *(vertexData + 0);
                    ptr0->mZ = center.mZ - *(vertexData + 2);

                    ptr1->mX = center.mX - *(vertexData + 3);
                    ptr1->mZ = center.mZ - *(vertexData + 5);

                    ptr2->mX = center.mX - *(vertexData + 6);
                    ptr2->mZ = center.mZ - *(vertexData + 8);
                    break;

                case kNorth:
                default:
                    ptr0->mX = center.mX + *(vertexData + 0);
                    ptr1->mX = center.mX + *(vertexData + 3);
                    ptr2->mX = center.mX + *(vertexData + 6);

                    ptr0->mZ = center.mZ + *(vertexData + 2);
                    ptr1->mZ = center.mZ + *(vertexData + 5);
                    ptr2->mZ = center.mZ + *(vertexData + 8);
                    break;
            }

            ptr0->mY = center.mY + *(vertexData + 1);
            ptr1->mY = center.mY + *(vertexData + 4);
            ptr2->mY = center.mY + *(vertexData + 7);

            projectAllVertices(3);

            coords[0] = fixToInt(projectionVertices[0].second.mX);
            coords[1] = fixToInt(projectionVertices[0].second.mY);
            coords[2] = fixToInt(projectionVertices[1].second.mX);
            coords[3] = fixToInt(projectionVertices[1].second.mY);
            coords[4] = fixToInt(projectionVertices[2].second.mX);
            coords[5] = fixToInt(projectionVertices[2].second.mY);

            fillTriangle(&coords[0], colour);
            vertexData += 9;
        }
    } else {
        uint8_t *uvData = mesh->uvCoords;
        for (c = 0; c < count; ++c) {

            ptr0 = &projectionVertices[0].first;
            ptr1 = &projectionVertices[1].first;
            ptr2 = &projectionVertices[2].first;

            switch (rotation) {
                case kEast:
                    ptr0->mX = center.mX - *(vertexData + 2);
                    ptr0->mZ = center.mZ + *(vertexData + 0);

                    ptr1->mX = center.mX - *(vertexData + 5);
                    ptr1->mZ = center.mZ + *(vertexData + 3);

                    ptr2->mX = center.mX - *(vertexData + 8);
                    ptr2->mZ = center.mZ + *(vertexData + 6);
                    break;
                case kWest:
                    ptr0->mX = center.mX + *(vertexData + 2);
                    ptr0->mZ = center.mZ - *(vertexData + 0);

                    ptr1->mX = center.mX + *(vertexData + 5);
                    ptr1->mZ = center.mZ - *(vertexData + 3);

                    ptr2->mX = center.mX + *(vertexData + 8);
                    ptr2->mZ = center.mZ - *(vertexData + 6);
                    break;
                case kSouth:
                    ptr0->mX = center.mX - *(vertexData + 0);
                    ptr0->mZ = center.mZ - *(vertexData + 2);

                    ptr1->mX = center.mX - *(vertexData + 3);
                    ptr1->mZ = center.mZ - *(vertexData + 5);

                    ptr2->mX = center.mX - *(vertexData + 6);
                    ptr2->mZ = center.mZ - *(vertexData + 8);
                    break;

                case kNorth:
                default:
                    ptr0->mX = center.mX + *(vertexData + 0);
                    ptr1->mX = center.mX + *(vertexData + 3);
                    ptr2->mX = center.mX + *(vertexData + 6);

                    ptr0->mZ = center.mZ + *(vertexData + 2);
                    ptr1->mZ = center.mZ + *(vertexData + 5);
                    ptr2->mZ = center.mZ + *(vertexData + 8);
                    break;
            }

            ptr0->mY = center.mY + *(vertexData + 1);
            ptr1->mY = center.mY + *(vertexData + 4);
            ptr2->mY = center.mY + *(vertexData + 7);

            projectAllVertices(3);

            coords[0] = fixToInt(projectionVertices[0].second.mX);
            coords[1] = fixToInt(projectionVertices[0].second.mY);
            coords[2] = fixToInt(projectionVertices[1].second.mX);
            coords[3] = fixToInt(projectionVertices[1].second.mY);
            coords[4] = fixToInt(projectionVertices[2].second.mX);
            coords[5] = fixToInt(projectionVertices[2].second.mY);

            drawTexturedTriangle(&coords[0], uvData, mesh->texture, fixToInt(ptr0->mZ));

            uvData += 6;
            vertexData += 9;
        }
    }
}


void drawRightNear(const struct Vec3 center,
                   const FixP_t scale,
                   const struct Texture *texture,
                   const uint8_t mask,
                   const uint8_t repeatedTexture) {

    FixP_t halfScale = scale;
    const FixP_t textureScale = (repeatedTexture ? halfScale : FIXP_ONE);
    FixP_t depth = FIXP_ONE;
    struct Vec2 ulz0;
    struct Vec2 urz0;
    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec3 *ptr0;
    struct Vec3 *ptr1;
    struct Vec3 *ptr2;
    struct Vec3 *ptr3;

    if (center.mZ <= kMinZCull) {
        return;
    }

    if (mask & MASK_BEHIND) {

        ptr0 = &projectionVertices[0].first;
        ptr1 = &projectionVertices[1].first;

        ptr0->mX = center.mX - FIXP_ONE;
        ptr1->mX = center.mX + FIXP_ONE;

        ptr0->mY = center.mY - halfScale;
        ptr1->mY = center.mY + halfScale;

        ptr0->mZ = center.mZ - FIXP_ONE;
        ptr1->mZ = center.mZ - FIXP_ONE;

        projectAllVertices(2);

        drawMask(projectionVertices[0].second.mX, projectionVertices[0].second.mY,
                 projectionVertices[1].second.mX, projectionVertices[1].second.mY);

        return;
    }

    if (cameraDirection == kWest || cameraDirection == kEast) {
        depth = -FIXP_ONE;
    }

    ptr0 = &projectionVertices[0].first;
    ptr1 = &projectionVertices[1].first;
    ptr2 = &projectionVertices[2].first;
    ptr3 = &projectionVertices[3].first;

    ptr0->mX = ptr2->mX = center.mX - FIXP_ONE;
    ptr1->mX = ptr3->mX = center.mX + FIXP_ONE;

    ptr0->mY = ptr1->mY = center.mY + halfScale;
    ptr2->mY = ptr3->mY = center.mY - halfScale;

    ptr0->mZ = ptr2->mZ = center.mZ + depth;
    ptr1->mZ = ptr3->mZ = center.mZ - depth;

    projectAllVertices(4);

    ulz0 = projectionVertices[0].second;
    urz0 = projectionVertices[1].second;
    llz0 = projectionVertices[2].second;
    lrz0 = projectionVertices[3].second;

    if (center.mZ >= FIXP_DISTANCE_FOR_DARKNESS) {
        maskWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY);
    } else {
        drawWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY, texture->rowMajor,
                 textureScale, fixToInt(center.mZ));
    }
}
