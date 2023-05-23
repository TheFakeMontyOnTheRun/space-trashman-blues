#include <string.h>
#include <stdlib.h>

#include <assert.h>
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif
#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "Globals.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"

#define kMinZCull 0
#define FIXP_HALF_XRES  (intToFix(HALF_XRES))
#define FIXP_HALF_YRES  (intToFix(HALF_YRES))
#define FIXP_ONE (intToFix(1))

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

			oneOver = Div(FIXP_HALF_YRES, z);
		}

		vertex->second.mX = (FIXP_HALF_XRES + Mul(vertex->first.mX + xCameraOffset, oneOver));
		vertex->second.mY = (FIXP_HALF_YRES - Mul(vertex->first.mY + compoundYFactor, oneOver));
	}
}

void drawBillboardAt(const struct Vec3 center,
                     const uint8_t *__restrict__ texture,
                     const FixP_t scale,
                     const int size) {
    FixP_t one = intToFix(1);
    FixP_t zero = 0;
    FixP_t minusOne = -one;
    FixP_t minusScale = (-scale);
    FixP_t halfScale = Div(scale, intToFix(2));
    struct Vec3 scaledCenter;
    struct Vec2 ulz0;
    struct Vec2 lrz0;
    int z = fixToInt(center.mZ);

    if (center.mZ <= kMinZCull) {
        return;
    }

    initVec3(&scaledCenter, center.mX, (center.mY), center.mZ);
    initVec3(&projectionVertices[0].first, scaledCenter.mX, scaledCenter.mY,
             scaledCenter.mZ);
    initVec3(&projectionVertices[1].first, scaledCenter.mX, scaledCenter.mY,
             scaledCenter.mZ);
    addToVec3(&projectionVertices[0].first, minusOne, scale, zero);
    addToVec3(&projectionVertices[1].first, one, minusScale, zero);

    projectAllVertices(2);

    ulz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;

    if (z >= distanceForDarkness && useDither) {
        drawMask(ulz0.mX, ulz0.mY, lrz0.mX, lrz0.mY);
    } else {
        drawFrontWall(ulz0.mX, ulz0.mY, lrz0.mX, lrz0.mY, texture,
                      (halfScale * 2), z, TRUE, size);
    }
}

void drawColumnAt(const struct Vec3 center,
                  const FixP_t scale,
                  const struct Texture *__restrict__ texture,
                  const uint8_t mask,
                  const int enableAlpha,
                  const int repeatTexture) {

    FixP_t one = intToFix(1);
    FixP_t minusOne = -one;
    const FixP_t halfScale = scale;
    const FixP_t minusHalfScale = (-scale);
    const FixP_t textureScale = (repeatTexture ? halfScale : one);
    struct Vec3 scaledCenter;
    struct Vec2 p0;
    struct Vec2 p1;
    struct Vec2 p2;
    struct Vec2 p3;
    int z = fixToInt(center.mZ);
    int originalZ = z;

    if (center.mZ <= kMinZCull) {
        return;
    }

    initVec3(&scaledCenter, center.mX, center.mY, center.mZ);

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

	projectionVertices[0].first = projectionVertices[1].first = projectionVertices[2].first = projectionVertices[3].first = scaledCenter;

    addToVec3(&projectionVertices[0].first, minusOne, halfScale, minusOne);
    addToVec3(&projectionVertices[1].first, one, minusHalfScale, minusOne);
    addToVec3(&projectionVertices[2].first, minusOne, halfScale, one);
    addToVec3(&projectionVertices[3].first, one, minusHalfScale, one);

    projectAllVertices(4);

    p0 = projectionVertices[0].second;
    p1 = projectionVertices[1].second;
    p2 = projectionVertices[2].second;
    p3 = projectionVertices[3].second;

    if ( (mask & MASK_BEHIND) || (enableAlpha && (mask & MASK_FRONT))) {
        if (z >= distanceForDarkness && useDither) {
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

        if (z >= distanceForDarkness && useDither) {
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

        if (z >= distanceForDarkness && useDither) {
            maskWall(p1.mX, p3.mX, p0.mY, p1.mY, p2.mY, p3.mY);
        } else {
            drawWall(p1.mX, p3.mX, p0.mY, p1.mY, p2.mY, p3.mY, texture->rowMajor,
                     (textureScale), z);
        }

        z = originalZ;
    }

    if ( (mask & MASK_FRONT)) {
        if (mask & ~MASK_BEHIND) {
            z -= 2;
        }

        if (z >= distanceForDarkness && useDither) {
            drawMask(p0.mX, p0.mY, p1.mX, p1.mY);
        } else {
            drawFrontWall(p0.mX, p0.mY, p1.mX, p1.mY, texture->rotations[0],
                          (textureScale), z, enableAlpha, NATIVE_TEXTURE_SIZE);
        }
    }
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
                const struct Texture *__restrict__ texture, uint8_t cameraDirection, uint8_t flipTexture) {

    FixP_t one = intToFix(1);
    FixP_t zero = 0;
    FixP_t minusOne = -one;
    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec2 llz1;
    struct Vec2 lrz1;
    int z;
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

        addToVec3(&projectionVertices[0].first, minusOne, zero, minusOne);
        addToVec3(&projectionVertices[1].first,      one, zero, minusOne);
        addToVec3(&projectionVertices[2].first, minusOne, zero,      one);
        addToVec3(&projectionVertices[3].first,      one, zero,      one);

        projectAllVertices(4);

        llz0 = projectionVertices[0].second;
        lrz0 = projectionVertices[1].second;
        llz1 = projectionVertices[2].second;
        lrz1 = projectionVertices[3].second;

        if (flipTexture) {
            uvCoords[0] = 0;
            uvCoords[1] = 32;
            uvCoords[2] = 0;
            uvCoords[3] = 0;
            uvCoords[4] = 32;
            uvCoords[5] = 32;
        } else {
            uvCoords[0] = 32;
            uvCoords[1] = 0;
            uvCoords[2] = 32;
            uvCoords[3] = 32;
            uvCoords[4] = 0;
            uvCoords[5] = 0;
        }

        coords[0] = fixToInt(llz1.mX); // 2
        coords[1] = fixToInt(llz1.mY);
        coords[2] = fixToInt(lrz1.mX); // 3
        coords[3] = fixToInt(lrz1.mY);
        coords[4] = fixToInt(llz0.mX); // 0
        coords[5] = fixToInt(llz0.mY);

        drawTexturedTriangle(&coords[0], &uvCoords[0], (struct Texture*)texture );


        if (flipTexture) {
            uvCoords[0] = 32;
            uvCoords[1] = 32;
            uvCoords[2] = 0;
            uvCoords[3] = 0;
            uvCoords[4] = 32;
            uvCoords[5] = 0;
        } else {
            uvCoords[0] = 0;
            uvCoords[1] = 0;
            uvCoords[2] = 32;
            uvCoords[3] = 32;
            uvCoords[4] = 0;
            uvCoords[5] = 32;
        }

        coords[0] = fixToInt(llz0.mX); //0
        coords[1] = fixToInt(llz0.mY);
        coords[2] = fixToInt(lrz1.mX); //3
        coords[3] = fixToInt(lrz1.mY);
        coords[4] = fixToInt(lrz0.mX); //1
        coords[5] = fixToInt(lrz0.mY);

        drawTexturedTriangle( &coords[0], &uvCoords[0], (struct Texture*)texture );

        return;
    }




    addToVec3(&projectionVertices[0].first, minusOne, zero, minusOne);
    addToVec3(&projectionVertices[1].first, one, zero, minusOne);
    addToVec3(&projectionVertices[2].first, minusOne, zero, one);
    addToVec3(&projectionVertices[3].first, one, zero, one);

    projectAllVertices(4);

    llz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;
    llz1 = projectionVertices[2].second;
    lrz1 = projectionVertices[3].second;

    z = fixToInt(p0.mZ);

    if (z >= distanceForDarkness && useDither) {
        maskFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, 0 );
    } else {
        drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, z, texture->rotations[cameraDirection] );
    }
}

void drawFloorAt(const struct Vec3 center,
                 const struct Texture *__restrict__ texture, uint8_t cameraDirection) {

    FixP_t one = intToFix(1);
    FixP_t zero = 0;
    FixP_t minusOne = -one;
    FixP_t threshold = 0;
    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec2 llz1;
    struct Vec2 lrz1;

    if (center.mZ <= kMinZCull) {
        return;
    }

	projectionVertices[0].first = projectionVertices[1].first = projectionVertices[2].first = projectionVertices[3].first = center;

    addToVec3(&projectionVertices[0].first, minusOne, zero, minusOne);
    addToVec3(&projectionVertices[1].first, one, zero, minusOne);
    addToVec3(&projectionVertices[2].first, minusOne, zero, one);
    addToVec3(&projectionVertices[3].first, one, zero, one);

    projectAllVertices(4);

    llz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;
    llz1 = projectionVertices[2].second;
    lrz1 = projectionVertices[3].second;

    if (center.mY <= threshold) {

        int z = fixToInt(center.mZ);

        if (z >= distanceForDarkness && useDither) {
            maskFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, 0);
        } else {
            drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, z,
                      texture->rotations[cameraDirection]);
        }
    }
}

void drawCeilingAt(const struct Vec3 center,
                   const struct Texture *__restrict__ texture, uint8_t cameraDirection) {

    FixP_t one = intToFix(1);
    FixP_t minusOne = -one;
    FixP_t zero = 0;
    FixP_t threshold = zero;
    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec2 llz1;
    struct Vec2 lrz1;

    if (center.mZ <= kMinZCull) {
        return;
    }

	projectionVertices[0].first = projectionVertices[1].first = projectionVertices[2].first = projectionVertices[3].first = center;

    addToVec3(&projectionVertices[0].first, minusOne, zero, minusOne);
    addToVec3(&projectionVertices[1].first, one, zero, minusOne);
    addToVec3(&projectionVertices[2].first, minusOne, zero, one);
    addToVec3(&projectionVertices[3].first, one, zero, one);

    projectAllVertices(4);

    llz0 = projectionVertices[0].second;
    lrz0 = projectionVertices[1].second;
    llz1 = projectionVertices[2].second;
    lrz1 = projectionVertices[3].second;

    if (center.mY >= threshold) {

        int z = fixToInt(center.mZ);

        if (z >= distanceForDarkness && useDither) {
            maskFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, 0);
        } else {
            drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, z,
                      texture->rotations[cameraDirection]);
        }
    }
}

void drawLeftNear(const struct Vec3 center,
                  const FixP_t scale,
                  const uint8_t *__restrict__ texture,
                  const uint8_t mask,
                  const int repeatTexture) {

    FixP_t one = intToFix(1);
    FixP_t minusOne = -one;
    FixP_t halfScale = scale;
    FixP_t minusHalfScale = (-scale);
    const FixP_t textureScale = (repeatTexture ? halfScale : one);
    FixP_t depth = one;
    FixP_t minusDepth = minusOne;
    struct Vec2 ulz0;
    struct Vec2 urz0;
    struct Vec2 llz0;
    struct Vec2 lrz0;
    int z = fixToInt(center.mZ);

    if (center.mZ <= kMinZCull) {
        return;
    }

    if (mask & MASK_BEHIND) {

		projectionVertices[0].first = projectionVertices[1].first = center;

        addToVec3(&projectionVertices[0].first, minusOne, minusHalfScale, minusOne);
        addToVec3(&projectionVertices[1].first, one, halfScale, minusOne);

        projectAllVertices(2);

        drawMask(projectionVertices[0].second.mX, projectionVertices[0].second.mY,
                 projectionVertices[1].second.mX, projectionVertices[1].second.mY);

        return;
    }

    if (cameraDirection == kWest || cameraDirection == kEast) {
        depth = minusOne;
        minusDepth = one;
    }

	projectionVertices[0].first = projectionVertices[1].first = projectionVertices[2].first = projectionVertices[3].first = center;

    addToVec3(&projectionVertices[0].first, minusOne, halfScale, minusDepth);
    addToVec3(&projectionVertices[1].first, one, halfScale, depth);
    addToVec3(&projectionVertices[2].first, minusOne, minusHalfScale, minusDepth);
    addToVec3(&projectionVertices[3].first, one, minusHalfScale, depth);

    projectAllVertices(4);

    ulz0 = projectionVertices[0].second;
    urz0 = projectionVertices[1].second;
    llz0 = projectionVertices[2].second;
    lrz0 = projectionVertices[3].second;

    if (z >= distanceForDarkness && useDither) {
        maskWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY);
    } else {
        drawWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY, texture,
                 textureScale, z);
    }
}

void drawMesh( const struct Mesh *mesh, const struct Vec3 center ) {

    int coords[6];
    int count = mesh->triangleCount;

    FixP_t * vertexData = mesh->geometry;
    uint8_t colour = mesh->colour;

    if (mesh->texture == NULL ) {
	    int c;
        for (c = 0; c < count; ++c ) {

            memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
            memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));
            memcpy (&projectionVertices[2].first, &center, sizeof(struct Vec3));

            addToVec3(&projectionVertices[0].first, *(vertexData + 0), *(vertexData + 1), *(vertexData + 2));
            addToVec3(&projectionVertices[1].first, *(vertexData + 3), *(vertexData + 4), *(vertexData + 5));
            addToVec3(&projectionVertices[2].first, *(vertexData + 6), *(vertexData + 7), *(vertexData + 8));

            projectAllVertices(3);

            coords[0] = fixToInt(projectionVertices[0].second.mX);
            coords[1] = fixToInt(projectionVertices[0].second.mY);
            coords[2] = fixToInt(projectionVertices[1].second.mX);
            coords[3] = fixToInt(projectionVertices[1].second.mY);
            coords[4] = fixToInt(projectionVertices[2].second.mX);
            coords[5] = fixToInt(projectionVertices[2].second.mY);

            fillTriangle( &coords[0], colour);
            vertexData += 9;
        }
    } else {
	    int c;
        uint8_t* uvData = mesh->uvCoords;
        for (c = 0; c < count; ++c ) {

            memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
            memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));
            memcpy (&projectionVertices[2].first, &center, sizeof(struct Vec3));

            addToVec3(&projectionVertices[0].first, *(vertexData + 0), *(vertexData + 1), *(vertexData + 2));
            addToVec3(&projectionVertices[1].first, *(vertexData + 3), *(vertexData + 4), *(vertexData + 5));
            addToVec3(&projectionVertices[2].first, *(vertexData + 6), *(vertexData + 7), *(vertexData + 8));

            projectAllVertices(3);

            coords[0] = fixToInt(projectionVertices[0].second.mX);
            coords[1] = fixToInt(projectionVertices[0].second.mY);
            coords[2] = fixToInt(projectionVertices[1].second.mX);
            coords[3] = fixToInt(projectionVertices[1].second.mY);
            coords[4] = fixToInt(projectionVertices[2].second.mX);
            coords[5] = fixToInt(projectionVertices[2].second.mY);

            drawTexturedTriangle( &coords[0], uvData, mesh->texture);
            uvData += 6;
            vertexData += 9;
        }
    }
}


void drawRightNear(const struct Vec3 center,
                   const FixP_t scale,
                   const uint8_t *__restrict__ texture,
                   const uint8_t mask,
                   const int repeatTexture) {

    FixP_t one = intToFix(1);
    FixP_t minusOne = -one;
    FixP_t halfScale = scale;
    FixP_t minusHalfScale = (-scale);
    const FixP_t textureScale = (repeatTexture ? halfScale : one);
    FixP_t depth = one;
    FixP_t minusDepth = minusOne;
    int z = fixToInt(center.mZ);
    struct Vec2 ulz0;
    struct Vec2 urz0;
    struct Vec2 llz0;
    struct Vec2 lrz0;

    if (center.mZ <= kMinZCull) {
        return;
    }

    if (mask & MASK_BEHIND) {

		projectionVertices[0].first = projectionVertices[1].first = center;

        addToVec3(&projectionVertices[0].first, minusOne, minusHalfScale, minusOne);
        addToVec3(&projectionVertices[1].first, one, halfScale, minusOne);

        projectAllVertices(2);

        drawMask(projectionVertices[0].second.mX, projectionVertices[0].second.mY,
                 projectionVertices[1].second.mX, projectionVertices[1].second.mY);

        return;
    }

    if (cameraDirection == kWest || cameraDirection == kEast) {

        depth = minusOne;
        minusDepth = one;
    }

	projectionVertices[0].first = projectionVertices[1].first = projectionVertices[2].first = projectionVertices[3].first = center;

    addToVec3(&projectionVertices[0].first, minusOne, halfScale, depth);
    addToVec3(&projectionVertices[1].first, one, halfScale, minusDepth);
    addToVec3(&projectionVertices[2].first, minusOne, minusHalfScale, depth);
    addToVec3(&projectionVertices[3].first, one, minusHalfScale, minusDepth);

    projectAllVertices(4);

    ulz0 = projectionVertices[0].second;
    urz0 = projectionVertices[1].second;
    llz0 = projectionVertices[2].second;
    lrz0 = projectionVertices[3].second;

    if (z >= distanceForDarkness && useDither) {
        maskWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY);
    } else {
        drawWall(ulz0.mX, urz0.mX, ulz0.mY, llz0.mY, urz0.mY, lrz0.mY, texture,
                 textureScale, z);
    }
}
