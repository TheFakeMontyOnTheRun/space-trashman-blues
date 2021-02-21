#include <string.h>
#include <stdlib.h>

#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "Globals.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"

#define kMinZCull 0

struct Projection3D2D {
    uint8_t px;
    uint8_t py;
    int16_t dx;
};


const struct Projection3D2D projections[80] =
/*
{
    //                                   Z
    {0,  127, -64}, // 0
    {0,  127, -64}, // 1
    {20, 106, -43}, // 2
    {31, 95,  -32}, // 3
    {37, 89,  -26}, // 4
    {42, 84,  -21}, // 5
    {45, 81,  -18},  // 6
    {47, 79,  -16},  // 7
    {49, 77,  -14},  // 8
    {50, 76,  -13},  // 9
    {51, 75,  -12},  // 10
    {52, 74,  -11},  // 11
    {53, 73,  -10}, // 12
    {54, 72,  -9}, // 13
    {54, 72,  -9}, // 14
    {55, 71,  -8}, // 15
    {55, 71,  -8}, // 16
    {56, 71,  -8}, // 17
    {56, 70,  -7}, // 18
    {57, 70,  -7}, // 19
    {57, 69,  -6}, // 20
    {57, 69,  -6}, // 21
    {57, 69,  -6},  // 22
    {58, 69,  -6},  // 23
    {58, 68,  -5},  // 24
    {58, 68,  -5},  // 25
    {58, 68,  -5},  // 26
    {58, 68,  -5},  // 27
    {59, 67,  -5}, // 28
    {59, 67,  -4}, // 29
    {59, 67,  -4}, // 30
    {59, 67,  -4}, // 31
};
*/
{
 {    -128    ,    192    ,    -256    },    //    0
 {    -128    ,    192    ,    -128    },    //    1
 {    0    ,    128    ,    -85    },    //    2
 {    43    ,    107    ,    -64    },    //    3
 {    64    ,    96    ,    -51    },    //    4
 {    77    ,    90    ,    -43    },    //    5
 {    85    ,    85    ,    -37    },    //    6
 {    91    ,    82    ,    -32    },    //    7
 {    96    ,    80    ,    -28    },    //    8
 {    100    ,    78    ,    -26    },    //    9
 {    102    ,    77    ,    -23    },    //    10
 {    105    ,    76    ,    -21    },    //    11
 {    107    ,    75    ,    -20    },    //    12
 {    108    ,    74    ,    -18    },    //    13
 {    110    ,    73    ,    -17    },    //    14
 {    111    ,    73    ,    -16    },    //    15
 {    112    ,    72    ,    -15    },    //    16
 {    113    ,    72    ,    -14    },    //    17
 {    114    ,    71    ,    -13    },    //    18
 {    115    ,    71    ,    -13    },    //    19
 {    115    ,    70    ,    -12    },    //    20
 {    116    ,    70    ,    -12    },    //    21
 {    116    ,    70    ,    -11    },    //    22
 {    117    ,    70    ,    -11    },    //    23
 {    117    ,    69    ,    -10    },    //    24
 {    118    ,    69    ,    -10    },    //    25
 {    118    ,    69    ,    -9    },    //    26
 {    119    ,    69    ,    -9    },    //    27
 {    119    ,    69    ,    -9    },    //    28
 {    119    ,    68    ,    -9    },    //    29
 {    119    ,    68    ,    -8    },    //    30
 {    120    ,    68    ,    -8    },    //    31
 {    120    ,    68    ,    -8    },    //    32
 {    120    ,    68    ,    -8    },    //    33
 {    120    ,    68    ,    -7    },    //    34
 {    121    ,    68    ,    -7    },    //    35
 {    121    ,    68    ,    -7    },    //    36
 {    121    ,    67    ,    -7    },    //    37
 {    121    ,    67    ,    -7    },    //    38
 {    121    ,    67    ,    -6    },    //    39
 {    122    ,    67    ,    -6    },    //    40
 {    122    ,    67    ,    -6    },    //    41
 {    122    ,    67    ,    -6    },    //    42
 {    122    ,    67    ,    -6    },    //    43
 {    122    ,    67    ,    -6    },    //    44
 {    122    ,    67    ,    -6    },    //    45
 {    122    ,    67    ,    -5    },    //    46
 {    123    ,    67    ,    -5    },    //    47
 {    123    ,    67    ,    -5    },    //    48
 {    123    ,    67    ,    -5    },    //    49
 {    123    ,    67    ,    -5    },    //    50
 {    123    ,    67    ,    -5    },    //    51
 {    123    ,    66    ,    -5    },    //    52
 {    123    ,    66    ,    -5    },    //    53
 {    123    ,    66    ,    -5    },    //    54
 {    123    ,    66    ,    -5    },    //    55
 {    123    ,    66    ,    -4    },    //    56
 {    124    ,    66    ,    -4    },    //    57
 {    124    ,    66    ,    -4    },    //    58
 {    124    ,    66    ,    -4    },    //    59
 {    124    ,    66    ,    -4    },    //    60
 {    124    ,    66    ,    -4    },    //    61
 {    124    ,    66    ,    -4    },    //    62
 {    124    ,    66    ,    -4    },    //    63
 {    124    ,    66    ,    -4    },    //    64
 {    124    ,    66    ,    -4    },    //    65
 {    124    ,    66    ,    -4    },    //    66
 {    124    ,    66    ,    -4    },    //    67
 {    124    ,    66    ,    -4    },    //    68
 {    124    ,    66    ,    -4    },    //    69
 {    124    ,    66    ,    -4    },    //    70
 {    124    ,    66    ,    -4    },    //    71
 {    124    ,    66    ,    -4    },    //    72
 {    124    ,    66    ,    -3    },    //    73
 {    125    ,    66    ,    -3    },    //    74
 {    125    ,    66    ,    -3    },    //    75
 {    125    ,    66    ,    -3    },    //    76
 {    125    ,    66    ,    -3    },    //    77
 {    125    ,    66    ,    -3    },    //    78
 {    125    ,    66    ,    -3    },    //    79
};
void projectAllVertices(const uint8_t count) {
    FixP_t halfWidth = intToFix(HALF_XRES);
    FixP_t halfHeight = intToFix(HALF_YRES);
    FixP_t zero = 0;
    FixP_t one = intToFix(1);
    FixP_t two = intToFix(2);
    FixP_t bias = Div(one, intToFix(128));
    FixP_t projected;
    FixP_t oneOver;
    int c;
    for (c = 0; c < count; ++c) {
        struct Projection *vertex = &projectionVertices[c];

        FixP_t z = (vertex->first.mZ);
        z += zCameraOffset;
        
        if (z < one) {
            z = one;
        }

        int intZ = fixToInt(z);
        
        if ( renderingMethod == FIXED )
        {
            intZ += zCameraOffset;

            projected = Div(z, two);
            
            if (projected == zero) {
                projected += bias;
            }
            
            oneOver = Div(halfHeight, projected);
            
            vertex->second.mX = (halfWidth + Mul(vertex->first.mX + xCameraOffset, oneOver));
            vertex->second.mY =
            (halfHeight
             - Mul(vertex->first.mY + playerHeight + walkingBias + yCameraOffset, oneOver));
            
            continue;
        }
        
        intZ = (intZ) * 2;
        
        if (intZ > 79 || intZ < 0 ) {
            return;
        }
        
        int x0 = 1 + ( fixToInt(vertex->first.mX));
        int y0 = 1 + (fixToInt(vertex->first.mY));
        int z0px = (projections[intZ].px);
        int z0dx = ((projections[intZ].dx));
        int px0z0 = z0px - (x0 * z0dx * 2);
        int z0py = (projections[intZ].py);
        int py0z0 = z0py + ((y0) * z0dx * 2);
        
        
        vertex->second.mX = intToFix(px0z0);
        vertex->second.mY = intToFix(py0z0);

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

  */
    memcpy (&projectionVertices[0].first, &scaledCenter, sizeof(struct Vec3));
    memcpy (&projectionVertices[1].first, &scaledCenter, sizeof(struct Vec3));
    memcpy (&projectionVertices[2].first, &scaledCenter, sizeof(struct Vec3));
    memcpy (&projectionVertices[3].first, &scaledCenter, sizeof(struct Vec3));

    addToVec3(&projectionVertices[0].first, minusOne, halfScale, minusOne);
    addToVec3(&projectionVertices[1].first, one, minusHalfScale, minusOne);
    addToVec3(&projectionVertices[2].first, minusOne, halfScale, one);
    addToVec3(&projectionVertices[3].first, one, minusHalfScale, one);

    projectAllVertices(4);

    p0 = projectionVertices[0].second;
    p1 = projectionVertices[1].second;
    p2 = projectionVertices[2].second;
    p3 = projectionVertices[3].second;

    if (enableAlpha && (mask & MASK_FRONT)) {
        if (z >= distanceForDarkness && useDither) {
            drawMask(p2.mX, p2.mY, p3.mX, p3.mY);
        } else {
            drawFrontWall(p2.mX, p2.mY, p3.mX, p3.mY, texture->rotations[0],
                          (textureScale), z, enableAlpha, NATIVE_TEXTURE_SIZE);
        }
    }

    if ((mask & MASK_RIGHT) && fixToInt(center.mX) > 0) {
        if (z >= distanceForDarkness && useDither) {
            maskWall(p2.mX, p0.mX, p2.mY, p3.mY, p0.mY, p1.mY);
        } else {
            drawWall(p2.mX, p0.mX, p2.mY, p3.mY, p0.mY, p1.mY, texture->rowMajor,
                     (textureScale), z);
        }
    }

    if ((mask & MASK_LEFT) && fixToInt(center.mX) < 0) {
        if (z >= distanceForDarkness && useDither) {
            maskWall(p1.mX, p3.mX, p0.mY, p1.mY, p2.mY, p3.mY);
        } else {
            drawWall(p1.mX, p3.mX, p0.mY, p1.mY, p2.mY, p3.mY, texture->rowMajor,
                     (textureScale), z);
        }
    }

    if ((mask & MASK_BEHIND) || (mask & MASK_FRONT)) {
        if (z >= distanceForDarkness && useDither) {
            drawMask(p0.mX, p0.mY, p1.mX, p1.mY);
        } else {
            drawFrontWall(p0.mX, p0.mY, p1.mX, p1.mY, texture->rotations[0],
                          (textureScale), z, enableAlpha, NATIVE_TEXTURE_SIZE);
        }
    }
}

void drawRampAt(const struct Vec3 p0, const struct Vec3 p1,
                const struct Texture *__restrict__ texture, uint8_t cameraDirection) {
    
    FixP_t one = intToFix(1);
    FixP_t zero = 0;
    FixP_t minusOne = -one;
    FixP_t threshold = 0;
    struct Vec2 llz0;
    struct Vec2 lrz0;
    struct Vec2 llz1;
    struct Vec2 lrz1;
    
    if (min(p0.mZ, p1.mZ) <= kMinZCull) {
        return;
    }
    
    if (cameraDirection == kNorth) {
        
        memcpy (&projectionVertices[0].first, &p0, sizeof(struct Vec3));
        memcpy (&projectionVertices[1].first, &p0, sizeof(struct Vec3));
        memcpy (&projectionVertices[2].first, &p1, sizeof(struct Vec3));
        memcpy (&projectionVertices[3].first, &p1, sizeof(struct Vec3));

    } else if (cameraDirection == kSouth) {
        
        memcpy (&projectionVertices[0].first, &p1, sizeof(struct Vec3));
        memcpy (&projectionVertices[1].first, &p1, sizeof(struct Vec3));
        memcpy (&projectionVertices[2].first, &p0, sizeof(struct Vec3));
        memcpy (&projectionVertices[3].first, &p0, sizeof(struct Vec3));

    } else {
        if (cameraDirection == kEast) {
            
            memcpy (&projectionVertices[0].first, &p1, sizeof(struct Vec3));
            memcpy (&projectionVertices[1].first, &p0, sizeof(struct Vec3));
            memcpy (&projectionVertices[2].first, &p1, sizeof(struct Vec3));
            memcpy (&projectionVertices[3].first, &p0, sizeof(struct Vec3));
        } else {
            
            memcpy (&projectionVertices[0].first, &p0, sizeof(struct Vec3));
            memcpy (&projectionVertices[1].first, &p1, sizeof(struct Vec3));
            memcpy (&projectionVertices[2].first, &p0, sizeof(struct Vec3));
            memcpy (&projectionVertices[3].first, &p1, sizeof(struct Vec3));
            
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
        
        uint8_t uvCoords[6];
        int coords[6];
        

        uvCoords[0] = 32;
        uvCoords[1] = 0;
        uvCoords[2] = 32;
        uvCoords[3] = 32;
        uvCoords[4] = 0;
        uvCoords[5] = 0;
        
        coords[0] = fixToInt(llz1.mX); // 2
        coords[1] = fixToInt(llz1.mY);
        coords[2] = fixToInt(lrz1.mX); // 3
        coords[3] = fixToInt(lrz1.mY);
        coords[4] = fixToInt(llz0.mX); // 0
        coords[5] = fixToInt(llz0.mY);
        
        drawTexturedTriangle(&coords[0], &uvCoords[0], texture );


        uvCoords[0] = 0;
        uvCoords[1] = 0;
        uvCoords[2] = 32;
        uvCoords[3] = 32;
        uvCoords[4] = 0;
        uvCoords[5] = 32;

        coords[0] = fixToInt(llz0.mX); //0
        coords[1] = fixToInt(llz0.mY);
        coords[2] = fixToInt(lrz1.mX); //3
        coords[3] = fixToInt(lrz1.mY);
        coords[4] = fixToInt(lrz0.mX); //1
        coords[5] = fixToInt(lrz0.mY);

        drawTexturedTriangle( &coords[0], &uvCoords[0], texture );

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
    
    int z = fixToInt(p0.mZ);
        
    if (z >= distanceForDarkness && useDither) {
        drawFloor(llz1.mY, lrz0.mY, llz1.mX, lrz1.mX, llz0.mX, lrz0.mX, z, texture->rotations[cameraDirection] );
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

    memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[2].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[3].first, &center, sizeof(struct Vec3));

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

    memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[2].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[3].first, &center, sizeof(struct Vec3));

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

        memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
        memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));

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

    memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[2].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[3].first, &center, sizeof(struct Vec3));

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
    uint8_t* uvData = mesh->uvCoords;
    
    for (int c = 0; c < count; ++c ) {
        
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

        drawTexturedTriangle( &coords[0], uvData, nativeTextures[23] );
        uvData += 6;
        vertexData += 9;
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

        memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
        memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));

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

    memcpy (&projectionVertices[0].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[1].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[2].first, &center, sizeof(struct Vec3));
    memcpy (&projectionVertices[3].first, &center, sizeof(struct Vec3));

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
