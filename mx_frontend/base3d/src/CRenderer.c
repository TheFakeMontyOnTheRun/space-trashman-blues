#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "FixP.h"
#include "Vec.h"
#include "Common.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "Core.h"
#include "Engine.h"
#include "Derelict.h"
#include "Globals.h"
#include "UI.h"

#define WALKING_BIAS (Div(intToFix(1), intToFix(16)))
#define STANDARD_HEIGHT (Div(intToFix(230), intToFix(100)))

extern const char *focusItemName;
int hasSnapshot = FALSE;
int leanX, leanY, turning;
FixP_t playerHeight = 0;
FixP_t walkingBias = 0;
FixP_t playerHeightChangeRate = 0;
FixP_t playerHeightTarget = 0;
int visibilityCached = FALSE;
int needsToRedrawVisibleMeshes = TRUE;
struct MapWithCharKey occluders;
struct MapWithCharKey colliders;
struct MapWithCharKey enemySightBlockers;
struct Bitmap *defaultFont;

#ifndef AGS
uint8_t framebuffer[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];
uint8_t previousFrame[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];
uint32_t palette[256];
#else
uint8_t *framebuffer;
#endif

enum EDirection cameraDirection;
struct Vec3 mCamera;
long gameTicks = 0;
int dirtyLineY0 = 0;
int dirtyLineY1 = YRES_FRAMEBUFFER;
#ifndef AGS
const int distanceForPenumbra = 16;
#else
const int distanceForPenumbra = 8;
#endif
struct Bitmap *mapTopLevel = NULL;
struct MapWithCharKey tileProperties;
struct Vec2i cameraPosition;
uint8_t texturesUsed = 0;
enum ECommand mBufferedCommand = kCommandNone;
struct Texture *nativeTextures[TOTAL_TEXTURES];
struct Bitmap *itemSprites[TOTAL_ITEMS];
int turnTarget = 0;
int turnStep = 0;
FixP_t xCameraOffset;
FixP_t yCameraOffset;
FixP_t zCameraOffset;
uint8_t enableSmoothMovement = FALSE;

struct Projection projectionVertices[8];

enum EVisibility *visMap;
struct Vec2i *distances;

char *messageLogBuffer;

int messageLogBufferCoolDown = 0;

void printMessageTo3DView(const char *message);

void enter2D() {

}

void enter3D() {

}

void printMessageTo3DView(const char *message) {
    strcpy(messageLogBuffer, message);
    messageLogBufferCoolDown = 5000;
}

void loadTileProperties(const uint8_t levelNumber) {
    char buffer[64];
    struct StaticBuffer data;
    int16_t c;

    setLoggerDelegate(printMessageTo3DView);

    clearMap(&occluders);
    clearMap(&colliders);
    clearMap(&enemySightBlockers);

    sprintf (buffer, "props%d.bin", levelNumber);

    data = loadBinaryFileFromPath(buffer);
    loadPropertyList(&buffer[0], &tileProperties);

    for (c = 0; c < 256; ++c) {
        struct CTile3DProperties *prop =
                (struct CTile3DProperties *) getFromMap(&tileProperties, c);

        if (prop) {

            if (prop->mBlockVisibility) {
                setInMap(&occluders, c, &occluders);
            }

            setInMap(&enemySightBlockers, c,
                     prop->mBlockEnemySight ? &enemySightBlockers : NULL);
        } else {
            setInMap(&occluders, c, NULL);
            setInMap(&enemySightBlockers, c, NULL);
        }
    }

    disposeDiskBuffer(data);
}

void loadTexturesForLevel(const uint8_t levelNumber) {
    char tilesFilename[64];
    struct StaticBuffer data;
    char *head;
    char *end;
    char *nameStart;
    char *buffer;

    sprintf (tilesFilename, "tiles%d.lst", levelNumber);

    data = loadBinaryFileFromPath(tilesFilename);
    buffer = (char *) allocMem(data.size, GENERAL_MEMORY, 1);
    head = buffer;
    memCopyToFrom(head, (void *) data.data, data.size);
    end = head + data.size;
    disposeDiskBuffer(data);

    nameStart = head;

    texturesUsed = 0;
    clearTextures();

    while (head != end && (texturesUsed < TOTAL_TEXTURES)) {
        char val = *head;
        if (val == '\n' || val == 0) {
            *head = 0;
            nativeTextures[texturesUsed] = (makeTextureFrom(nameStart));
            nameStart = head + 1;
            texturesUsed++;
        }
        ++head;
    }

    disposeMem(buffer);
}

void updateCursorForRenderer(const int x, const int z) {
#ifndef AGS
    needsToRedrawVisibleMeshes = TRUE;
    visibilityCached = FALSE;
#endif
}

void drawMap(const struct CActor *current) {

    int8_t z, x;
    const struct Vec2i mapCamera = current->position;
    cameraDirection = current->rotation;
    hasSnapshot = TRUE;

    if (!enable3DRendering) {
        return;
    }

    if (abs(yCameraOffset) <= 1000) {
        yCameraOffset = 0;
    }

    if (yCameraOffset > 0) {
        yCameraOffset -= Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if (zCameraOffset > 0) {
        zCameraOffset -= Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if (zCameraOffset < 0) {
        zCameraOffset += Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if (xCameraOffset > 0) {
        xCameraOffset -= Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if (xCameraOffset < 0) {
        xCameraOffset += Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if (yCameraOffset < 0) {
        yCameraOffset += Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    }

    if (visibilityCached) {
        return;
    }

    visibilityCached = TRUE;
    needsToRedrawVisibleMeshes = TRUE;

    cameraPosition = mapCamera;

    switch (cameraDirection) {
        case kNorth:
            mCamera.mX = intToFix(((MAP_SIZE - 1) * 2) - (2 * cameraPosition.x));
            mCamera.mZ = intToFix((2 * cameraPosition.y) - ((MAP_SIZE * 2) - 1));
            break;

        case kSouth:
            mCamera.mX = intToFix((2 * cameraPosition.x));
            mCamera.mZ = intToFix(-2 * cameraPosition.y - 1);
            break;

        case kWest:
            mCamera.mX = intToFix((2 * cameraPosition.y));
            mCamera.mZ = intToFix((2 * cameraPosition.x) - 1);
            break;

        case kEast:
            mCamera.mX = intToFix(-(2 * cameraPosition.y));
            mCamera.mZ = intToFix(((MAP_SIZE * 2) - 1) - (2 * cameraPosition.x));
            break;
    }

#ifndef FIX16
    if ((cameraPosition.x + cameraPosition.y) & 1) {
        walkingBias = WALKING_BIAS;
    } else {
        walkingBias = 0;
    }
#else
    walkingBias = 0;
#endif

    castVisibility(cameraDirection, visMap, cameraPosition,
                   distances, TRUE, &occluders);

    ++gameTicks;
}

enum ECommand getInput(void) {
    const enum ECommand toReturn = mBufferedCommand;
    mBufferedCommand = kCommandNone;
    return toReturn;
}

void render(const long ms) {

    if (messageLogBufferCoolDown > 0) {
        messageLogBufferCoolDown -= ms;
    }

    if (!enable3DRendering) {
        return;
    }

    if (!hasSnapshot) {
        return;
    }

    if (playerHeight < playerHeightTarget) {
        playerHeight += playerHeightChangeRate;
    }

    if (needsToRedrawVisibleMeshes) {
        uint8_t itemsSnapshotElement = 0xFF;
        struct Vec3 tmp, tmp2;
        struct CTile3DProperties *tileProp;
        FixP_t heightDiff;
        uint8_t lastElement = 0xFF;
        uint8_t element = 0;
        struct Vec3 position;
        FixP_t tileHeight = 0;
        int32_t x, z;
        int distance;
        FixP_t cameraHeight;
        uint8_t facesMask;

        needsToRedrawVisibleMeshes = FALSE;
#ifdef SDLSW
        clearRenderer();
#endif
        element = LEVEL_MAP(cameraPosition.x, cameraPosition.y);

        tileProp = ((struct CTile3DProperties *) getFromMap(&tileProperties,
                                                            element));

        if (tileProp) {
            tileHeight = tileProp->mFloorHeight;
        }

        cameraHeight = -2 * tileHeight;

        mCamera.mY = cameraHeight - STANDARD_HEIGHT;

        for (distance = (MAP_SIZE + MAP_SIZE - 1); distance >= 0; --distance) {
            uint8_t bucketPos;

            for (bucketPos = 0; bucketPos < MAP_SIZE; ++bucketPos) {

                struct Vec2i visPos = distances[(distance * MAP_SIZE) + bucketPos];

                if (visPos.x < 0 || visPos.y < 0 || visPos.x >= MAP_SIZE
                    || visPos.y >= MAP_SIZE) {
                    bucketPos = MAP_SIZE;
                    continue;
                }

                facesMask = MASK_LEFT | MASK_FRONT | MASK_RIGHT;

                switch (cameraDirection) {

                    case kNorth:
                        x = visPos.x;
                        z = visPos.y;
                        element = LEVEL_MAP(x, z);

                        itemsSnapshotElement = ITEMS_IN_MAP(x, z);

                        position.mX =
                                mCamera.mX + intToFix(-2 * ((MAP_SIZE - 1) - x));
                        position.mY = mCamera.mY;
                        position.mZ =
                                mCamera.mZ + intToFix(2 * (MAP_SIZE) - (2 * z));

                        if (x > 0) {
                            facesMask |= (LEVEL_MAP(x - 1, z) != element) ?
                                         MASK_RIGHT :
                                         0;
                        }

                        /* remember, bounds - 1! */
                        if ((x < (MAP_SIZE - 1)) && (LEVEL_MAP(x + 1, z) == element)) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((z < (MAP_SIZE - 1)) && (LEVEL_MAP(x, z + 1) == element)) {
                            facesMask &= ~MASK_FRONT;
                        }

                        if (z == cameraPosition.y - 1) {
                            if (getFromMap(&occluders, element)) {
                                facesMask &= ~MASK_FRONT;
                                facesMask |= MASK_BEHIND;
                            } else {
                                facesMask |= MASK_FRONT;
                            }
                        }

                        break;

                    case kSouth:
                        x = visPos.x;
                        z = visPos.y;

                        element = LEVEL_MAP(x, z);
                        itemsSnapshotElement = ITEMS_IN_MAP(x, z);

                        position.mX = mCamera.mX + intToFix(-2 * x);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(2 + 2 * z);

                        /*						remember, bounds - 1!*/

                        if ((x > 0) && (LEVEL_MAP(x - 1, z) == element)) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((x < (MAP_SIZE - 1)) && (LEVEL_MAP(x + 1, z) == element)) {
                            facesMask &= ~MASK_RIGHT;
                        }

                        if (z == (cameraPosition.y) + 1) {

                            if (getFromMap(&occluders, element)) {
                                facesMask &= ~MASK_FRONT;
                                facesMask |= MASK_BEHIND;
                            } else {
                                facesMask |= MASK_FRONT;
                            }
                        }

                        break;
                    case kWest:
                        x = visPos.y;
                        z = visPos.x;

                        element = LEVEL_MAP(z, x);
                        itemsSnapshotElement = ITEMS_IN_MAP(z, x);

                        position.mX = mCamera.mX + intToFix(-2 * x);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(2 - 2 * z);

                        /* remember, bounds - 1! */

                        if ((x > 0) && (LEVEL_MAP(z, x - 1) == element)) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((x < (MAP_SIZE - 1)) && (LEVEL_MAP(z, x + 1) == element)) {
                            facesMask &= ~MASK_RIGHT;
                        }

                        if ((z < (MAP_SIZE - 1)) && (LEVEL_MAP(z + 1, x) == element)) {
                            facesMask &= ~MASK_FRONT;
                        }

                        if (z == (cameraPosition.x) - 1) {

                            if (getFromMap(&occluders, element)) {
                                facesMask &= ~MASK_FRONT;
                                facesMask |= MASK_BEHIND;
                            } else {
                                facesMask |= MASK_FRONT;
                            }
                        }
                        break;

                    case kEast:
                        x = visPos.y;
                        z = visPos.x;

                        /* yes, it's reversed */
                        element = LEVEL_MAP(z, x);
                        itemsSnapshotElement = ITEMS_IN_MAP(z, x);

                        position.mX = mCamera.mX + intToFix(2 * x);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(2 * (z - MAP_SIZE + 1));


                        /* remember, bounds - 1! */
                        if ((x > 0) && (LEVEL_MAP(z, x - 1) == element)) {
                            facesMask &= ~MASK_RIGHT;
                        }

                        if ((x < (MAP_SIZE - 1)) && (LEVEL_MAP(z, x + 1) == element)) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((z < (MAP_SIZE - 1)) && (LEVEL_MAP(z - 1, x) == element)) {
                            facesMask &= ~MASK_FRONT;
                        }

                        if (z == (cameraPosition.x) + 1) {

                            if (getFromMap(&occluders, element)) {
                                facesMask &= ~MASK_FRONT;
                                facesMask |= MASK_BEHIND;
                            } else {
                                facesMask |= MASK_FRONT;
                            }
                        }
                        break;
                    default:
                        assert (FALSE);
                }

                if (lastElement != element) {
                    tileProp = (struct CTile3DProperties *) getFromMap(&tileProperties,
                                                                       element);
                }

                if (tileProp == NULL) {
                    continue;
                }

                heightDiff = tileProp->mCeilingHeight - tileProp->mFloorHeight;
                lastElement = element;

                tmp.mX = tmp2.mX = position.mX;
                tmp.mZ = tmp2.mZ = position.mZ;

                if (tileProp->mFloorRepeatedTextureIndex != 0xFF
                    && tileProp->mFloorRepetitions > 0) {

                    switch (tileProp->mGeometryType) {
                        case kRightNearWall:
                            tmp.mY = position.mY +
                                     ((tileProp->mFloorHeight * 2) - intToFix(tileProp->mFloorRepetitions));
                            drawRightNear(
                                    tmp, intToFix(tileProp->mFloorRepetitions),
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex]
                                            ->rowMajor,
                                    facesMask, TRUE);

                            break;

                        case kLeftNearWall:
                            tmp.mY = position.mY +
                                     ((tileProp->mFloorHeight * 2) - intToFix(tileProp->mFloorRepetitions));
                            drawLeftNear(
                                    tmp, intToFix(tileProp->mFloorRepetitions),
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex]
                                            ->rowMajor, facesMask, TRUE);
                            break;

                        case kCube:
                        case kRampNorth:
                        case kRampEast:
                        default:
                            tmp.mY = position.mY +
                                     ((tileProp->mFloorHeight * 2) - intToFix(tileProp->mFloorRepetitions));
                            drawColumnAt(
                                    tmp, intToFix(tileProp->mFloorRepetitions),
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex],
                                    facesMask, FALSE, TRUE);
                            break;
                    }
                }

                if (tileProp->mCeilingRepeatedTextureIndex != 0xFF
                    && tileProp->mCeilingRepetitions > 0) {

                    switch (tileProp->mGeometryType) {
                        case kRightNearWall:
                            tmp.mY = position.mY +
                                     ((tileProp->mCeilingHeight * 2) + intToFix(tileProp->mCeilingRepetitions));
                            drawRightNear(
                                    tmp, intToFix(tileProp->mCeilingRepetitions),
                                    nativeTextures[tileProp->mCeilingRepeatedTextureIndex]
                                            ->rowMajor,
                                    facesMask, TRUE);
                            break;

                        case kLeftNearWall:
                            tmp.mY = position.mY +
                                     ((tileProp->mCeilingHeight * 2) + intToFix(tileProp->mCeilingRepetitions));
                            drawLeftNear(
                                    tmp, intToFix(tileProp->mCeilingRepetitions),
                                    nativeTextures[tileProp->mCeilingRepeatedTextureIndex]
                                            ->rowMajor,
                                    facesMask, TRUE);
                            break;

                        case kCube:
                        case kRampNorth:
                        case kRampEast:
                        default:
                            tmp.mY = position.mY +
                                     ((tileProp->mCeilingHeight * 2) + intToFix(tileProp->mCeilingRepetitions));
                            drawColumnAt(
                                    tmp, intToFix(tileProp->mCeilingRepetitions),
                                    nativeTextures[tileProp->mCeilingRepeatedTextureIndex],
                                    facesMask, FALSE, TRUE);
                            break;
                    }
                }

                if (tileProp->mFloorTextureIndex != 0xFF) {
                    tmp.mY = position.mY + (tileProp->mFloorHeight * 2);
                    drawFloorAt(tmp, nativeTextures[tileProp->mFloorTextureIndex], cameraDirection);
                }

                if (tileProp->mCeilingTextureIndex != 0xFF) {
                    uint8_t newDirection = cameraDirection;

                    tmp.mY = position.mY + (tileProp->mCeilingHeight * 2);

                    if (cameraDirection == kNorth) {
                        newDirection = kSouth;
                    }
                    if (cameraDirection == kSouth) {
                        newDirection = kNorth;
                    }

                    drawCeilingAt(
                            tmp, nativeTextures[tileProp->mCeilingTextureIndex], newDirection);
                }

                if (tileProp->mGeometryType != kNoGeometry
                    && tileProp->mMainWallTextureIndex != 0xFF) {

                    int integerPart = fixToInt(tileProp->mCeilingHeight)
                                      - fixToInt(tileProp->mFloorHeight);

                    FixP_t adjust = 0;

                    if (((heightDiff * 2) - intToFix(integerPart)) >= Div(intToFix(1), intToFix(2))) {
                        adjust = Div(Div(intToFix(1), intToFix(2)), intToFix(8));
                    }

                    switch (tileProp->mGeometryType) {
                        case kWallNorth:
                            tmp.mY = position.mY + ((tileProp->mFloorHeight * 2) + heightDiff);

                            switch (cameraDirection) {
                                case kNorth:
                                    facesMask = MASK_BEHIND;
                                    break;
                                case kWest:
                                    facesMask = MASK_FORCE_LEFT;
                                    break;
                                case kSouth:
                                    facesMask = MASK_FRONT;
                                    break;
                                case kEast:
                                    facesMask = MASK_FORCE_RIGHT;
                                    break;
                                default:
                                    facesMask = 0;
                                    break;
                            }

                            drawColumnAt(tmp, (heightDiff + adjust),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                            break;
                        case kWallWest:
                            tmp.mY = position.mY + ((tileProp->mFloorHeight * 2) + heightDiff);

                            switch (cameraDirection) {
                                case kNorth:
                                    facesMask = MASK_FORCE_RIGHT;
                                    break;
                                case kWest:
                                    facesMask = MASK_BEHIND;
                                    break;
                                case kSouth:
                                    facesMask = MASK_FORCE_LEFT;
                                    break;
                                case kEast:
                                    facesMask = MASK_FRONT;
                                    break;
                                default:
                                    facesMask = 0;
                                    break;
                            }

                            drawColumnAt(tmp, (heightDiff + adjust),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                            break;

                        case kWallCorner:
                            tmp.mY = position.mY + ((tileProp->mFloorHeight * 2) + heightDiff);

                            switch (cameraDirection) {
                                case kNorth:
                                    facesMask = MASK_BEHIND | MASK_FORCE_RIGHT;
                                    break;
                                case kWest:
                                    facesMask = MASK_FORCE_LEFT | MASK_BEHIND;
                                    break;
                                case kSouth:
                                    facesMask = MASK_FRONT | MASK_FORCE_LEFT;
                                    break;
                                case kEast:
                                    facesMask = MASK_FORCE_RIGHT | MASK_FRONT;
                                    break;
                                default:
                                    facesMask = 0;
                                    break;
                            }

                            drawColumnAt(tmp, (heightDiff + adjust),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                            break;

                        case kRightNearWall:
                            tmp.mY = position.mY + ((tileProp->mFloorHeight * 2) + heightDiff);

                            drawRightNear(
                                    tmp, (heightDiff + adjust),
                                    nativeTextures[tileProp->mMainWallTextureIndex]->rowMajor,
                                    facesMask, tileProp->mRepeatMainTexture);
                            break;

                        case kLeftNearWall:
                            tmp.mY = position.mY + ((tileProp->mFloorHeight * 2) + heightDiff);

                            drawLeftNear(
                                    tmp, (heightDiff + adjust),
                                    nativeTextures[tileProp->mMainWallTextureIndex]->rowMajor,
                                    facesMask, tileProp->mRepeatMainTexture);
                            break;

                        case kRampNorth: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mFloorHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mCeilingHeight * 2);

                            flipTextureVertical = (cameraDirection == kSouth || cameraDirection == kEast);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection,
                                       flipTextureVertical);
                        }
                            break;

                        case kRampSouth: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mCeilingHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mFloorHeight * 2);

                            flipTextureVertical = (cameraDirection == kSouth || cameraDirection == kWest);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection,
                                       flipTextureVertical);
                        }
                            break;

                        case kRampEast: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mCeilingHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mFloorHeight * 2);

                            flipTextureVertical = (cameraDirection == kSouth || cameraDirection == kEast);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex],
                                       (cameraDirection + 1) & 3, flipTextureVertical);
                        }
                            break;
                        case kRampWest: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mCeilingHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mFloorHeight * 2);

                            flipTextureVertical = (cameraDirection == kNorth || cameraDirection == kWest);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex],
                                       (cameraDirection + 3) & 3, flipTextureVertical);
                        }
                            break;
                        case kCube:
                            tmp.mY = position.mY + ((tileProp->mFloorHeight * 2) + heightDiff);
                            drawColumnAt(tmp, (heightDiff + adjust),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                        default:
                            break;
                    }
                }


                if (itemsSnapshotElement != 0xFF) {
                    tmp.mY = position.mY + (tileProp->mFloorHeight * 2) + intToFix(1);

                    /* lazy loading the item sprites */
                    if (itemSprites[itemsSnapshotElement] == NULL) {
                        char buffer[64];
                        sprintf(&buffer[0], "%s.img", getItem(itemsSnapshotElement)->name);
                        itemSprites[itemsSnapshotElement] = loadBitmap(&buffer[0]);
                    }

                    drawBillboardAt(tmp, itemSprites[itemsSnapshotElement]->data, intToFix(1), 32);
                }
            }
        }


        if (turnTarget == turnStep) {
            if (focusItemName != NULL) {
                size_t len = strlen(focusItemName);
                int lines = 1 + (len / 27);
                fill(0, YRES - (8 * lines), XRES, lines * 8, 0, 1);
                drawTextAtWithMarginWithFiltering(1, 26 - lines, XRES, focusItemName, 255, ' ');
            }


            if (currentPresentationState == kRoomTransitioning) {
                messageLogBufferCoolDown = 0;
            }

            if (messageLogBufferCoolDown > 0) {
                int len = strlen(messageLogBuffer);
                int lines = 1;
                int chars = 0;
                int c;

                for (c = 0; c < len; ++c) {

                    ++chars;

                    if (chars == 27 || messageLogBuffer[c] == '\n') {
                        chars = 0;
                        ++lines;
                    }
                }

                fill(0, 0, 216, lines * 8, 0, 1);

                drawTextAt(1, 1, messageLogBuffer, 255);
            }
        }

        clippingY1 = YRES_FRAMEBUFFER;

        dirtyLineY0 = 0;
        dirtyLineY1 = YRES_FRAMEBUFFER;

        if (needsToRedrawHUD) {
            needsToRedrawHUD = FALSE;
            redrawHUD();
        }

        updateMap();
    }
}

void loadMesh(struct Mesh *mesh, char *filename) {
    struct StaticBuffer buffer = loadBinaryFileFromPath(filename);
    FixP_t val = 0;
    const uint8_t *bufferHead = buffer.data;
    int16_t trigCount = 0;
    int uvCoordsCount;
    int coordsCount;
    char *textureName;
    uint8_t *uvCoord;
    FixP_t *coord;
    uint8_t read;
    int c;

    read = (*(bufferHead++));
    trigCount += read;
    read = (*(bufferHead++)) << 8;
    trigCount += read;

    uvCoordsCount = trigCount * 6;
    coordsCount = trigCount * 9;

    mesh->triangleCount = trigCount;
    mesh->uvCoords = allocMem(uvCoordsCount, GENERAL_MEMORY, 1);
    mesh->geometry = allocMem(sizeof(FixP_t) * coordsCount, GENERAL_MEMORY, 1);

    uvCoord = mesh->uvCoords;
    coord = mesh->geometry;

    for (c = 0; c < uvCoordsCount; ++c) {
        *(uvCoord++) = (*(bufferHead++));
    }

    for (c = 0; c < coordsCount; ++c) {
        val = 0;
        val += (*(bufferHead++) << 0);
        val += (*(bufferHead++) << 8);
        val += (*(bufferHead++) << 16);
        val += (*(bufferHead++) << 24);
        *(coord++) = val;
    }

    read = (*(bufferHead++));

    if (read == 0) {
        mesh->colour = *bufferHead;
        mesh->texture = NULL;
    } else {
        textureName = allocMem(read + 1, GENERAL_MEMORY, 1);
        memCopyToFrom(textureName, (void *) bufferHead, read);
        mesh->texture = makeTextureFrom(textureName);
        disposeMem(textureName);
    }

    disposeDiskBuffer(buffer);
}
