#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "CPackedFileReader.h"

#include "Derelict.h"

/*	Div(intToFix(1), intToFix(16));*/
#define WALKING_BIAS 4096

int hasSnapshot = FALSE;
FixP_t playerHeight = 0;
FixP_t walkingBias = 0;
FixP_t playerHeightChangeRate = 0;
FixP_t playerHeightTarget = 0;
int cursorX = -1;
int cursorZ = -1;

extern int currentSelectedItem;
int covered = FALSE;
int useDither = TRUE;
int visibilityCached = FALSE;
int needsToRedrawVisibleMeshes = TRUE;
struct MapWithCharKey occluders;
struct MapWithCharKey colliders;
struct MapWithCharKey enemySightBlockers;
uint8_t *visibleElementsMap;
struct Bitmap *defaultFont;
uint8_t framebuffer[320 * 200];
uint8_t previousFrame[320 * 200];
uint8_t mActors[MAP_SIZE][MAP_SIZE];
uint8_t mItems[MAP_SIZE][MAP_SIZE];
uint8_t mEffects[MAP_SIZE][MAP_SIZE];
enum EDirection cameraDirection;
extern int playerHealth;
struct Vec3 mCamera;
long gameTicks = 0;

int distanceForPenumbra = 24;
int distanceForDarkness = 48;

#ifdef SDLSW
int mSlow = FALSE;
#endif
int linesOfSight[MAP_SIZE][MAP_SIZE];
int revealed[MAP_SIZE][MAP_SIZE];
struct Bitmap *backdrop = NULL;
struct MapWithCharKey tileProperties;
struct Vec2i cameraPosition;
uint32_t palette[256];
uint8_t texturesUsed = 0;
enum ECommand mBufferedCommand = kCommandNone;
struct Texture *nativeTextures[TOTAL_TEXTURES];
struct Texture *itemSprites[32];
int turnTarget = 0;
int turnStep = 0;
FixP_t xCameraOffset;
FixP_t yCameraOffset;
FixP_t zCameraOffset;
char* focusItemName = NULL;

struct Projection projectionVertices[8];

int coords[6];

enum EVisibility visMap[MAP_SIZE * MAP_SIZE];
uint8_t intMap[MAP_SIZE * MAP_SIZE];
struct Vec2i distances[2 * MAP_SIZE * MAP_SIZE];

char messageLogBuffer[256];

int messageLogBufferCoolDown = 0;

void printMessageTo3DView(const char* message ) {
    strcpy(&messageLogBuffer[0], message);
    messageLogBufferCoolDown = 5000;
}

void loadTileProperties(const uint8_t levelNumber) {
    char buffer[64];
    uint8_t *data;
    int16_t c;
    
    setLoggerDelegate(printMessageTo3DView);
    

    clearMap(&tileProperties);
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

    free(data);
    
    
    
    coords[0] =  70;
    coords[1] =  55;
    coords[2] =  75;
    coords[3] = 170;
    coords[4] =  45;
    coords[5] =  100;
}

void loadTexturesForLevel(const uint8_t levelNumber) {
    char tilesFilename[64];
    uint8_t *data;
    size_t size;
    char *head;
    char *end;
    char *nameStart;

    sprintf (tilesFilename, "tiles%d.lst", levelNumber);

    data = loadBinaryFileFromPath(tilesFilename);
    size = sizeOfFile(tilesFilename);
    head = (char *) data;
    end = head + size;
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

    free(data);

    backdrop = loadBitmap("backdrop.img");
    
    
    char buffer[256];
    
    for ( int c = 0; c < 30; ++c ) {
        sprintf( &buffer[0], "%s.img",  getItem(c)->description);
        itemSprites[c] = (makeTextureFrom(&buffer[0]));
    }
}

void updateCursorForRenderer(const int x, const int z) {
    needsToRedrawVisibleMeshes = TRUE;
    visibilityCached = FALSE;
    cursorX = x;
    cursorZ = z;
}

void drawMap(const uint8_t * __restrict__ elements,
             const uint8_t * __restrict__ items,
             const uint8_t * __restrict__ actors,
             uint8_t * __restrict__ effects,
             const struct CActor *current) {

    int8_t z, x;
    const struct Vec2i mapCamera = current->mPosition;
    cameraDirection = current->mDirection;
    visibleElementsMap = elements;
    hasSnapshot = TRUE;

    if (!enable3DRendering) {
        return;
    }
    
    if (abs(yCameraOffset) <= 1000 ) {
        yCameraOffset = 0;
    }
    
    if ( yCameraOffset > 0 ){
        yCameraOffset -= Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if ( zCameraOffset > 0 ){
        zCameraOffset -= Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if ( zCameraOffset < 0 ){
        zCameraOffset += Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if ( xCameraOffset > 0 ){
        xCameraOffset -= Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if ( xCameraOffset < 0 ){
        xCameraOffset += Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    } else if ( yCameraOffset < 0 ){
        yCameraOffset += Div(intToFix(1), intToFix(2));
        needsToRedrawVisibleMeshes = TRUE;
    }



    if (visibilityCached) {
        return;
    }

    visibilityCached = TRUE;
    needsToRedrawVisibleMeshes = TRUE;

    cameraPosition = mapCamera;
    
    for (z = 0; z < MAP_SIZE; ++z) {
        for (x = 0; x < MAP_SIZE; ++x) {
            const uint16_t offset = (MAP_SIZE * z) + x;
            const uint8_t actor = actors[offset];
            const uint8_t item = items[offset];
            const uint8_t effect = effects[offset];

            mActors[z][x] = 0xFF;
            mItems[z][x] = 0xFF;
            mEffects[z][x] = 0xFF;

            if (actor != 0xFF) {
                mActors[z][x] = actor;
            }

            if (item != 0xFF) {
                mItems[z][x] = item;
            }

            if (effect != 0xFF) {
                mEffects[z][x] = effect;
            }
        }
    }

    switch (cameraDirection) {
        case kNorth:
            mCamera.mX = intToFix( ((MAP_SIZE - 1) * 2) - (2 * cameraPosition.x));
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
            mCamera.mZ = intToFix( ((MAP_SIZE * 2) - 1) - (2 * cameraPosition.x));
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

    castVisibility(cameraDirection, visMap, &elements[0], cameraPosition,
                   distances, TRUE, &occluders);

    ++gameTicks;
}

enum ECommand getInput() {
    const enum ECommand toReturn = mBufferedCommand;
    mBufferedCommand = kCommandNone;
    return toReturn;
}

void render(const long ms) {
    static FixP_t zero = 0;
    FixP_t two = intToFix(2);
    FixP_t four = intToFix(4);
    FixP_t one = intToFix(1);
    const FixP_t halfOne = Div(one, two);
    FixP_t standardHeight = Div(intToFix(195), intToFix(100));
    
    if (messageLogBufferCoolDown > 0 ) {
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
        uint8_t actorsSnapshotElement = 0xFF;
        uint8_t itemsSnapshotElement = 0xFF;
        uint8_t effectsSnapshotElement = 0xFF;
        char buffer[64];
        char directions[4] = {'N', 'E', 'S', 'W'};
        struct Vec3 tmp, tmp2;
        struct CTile3DProperties *tileProp;
        FixP_t heightDiff;
        uint8_t lastElement = 0xFF;
        uint8_t element = 0;
        int onTarget = FALSE;
        struct Vec3 position;
        FixP_t tileHeight = 0;
        int16_t x, y, z;
        int distance;
        FixP_t cameraHeight;
        int c;
        uint8_t facesMask;

        needsToRedrawVisibleMeshes = FALSE;
#ifdef SDLSW
        clearRenderer();
#endif

#ifndef CD32
        clippingY1 = 200;
        for (c = 0; c < (256 / 32); ++c) {
            drawBitmap(c * 32, 0, backdrop, FALSE);
        }
        fill(0, 100, 256, 100, 0, FALSE);
#else
        fill(0, 0, 256, 128, 64, FALSE);
#endif

        element = visibleElementsMap[(cameraPosition.y * MAP_SIZE) + cameraPosition.x];

        tileProp = ((struct CTile3DProperties *) getFromMap(&tileProperties,
                                                            element));

        if (tileProp) {
            tileHeight = tileProp->mFloorHeight;
        }

        cameraHeight = -2 * tileHeight;

        mCamera.mY = cameraHeight - standardHeight;

        for (distance = (MAP_SIZE + MAP_SIZE - 1); distance >= 0; --distance) {
            uint8_t bucketPos;

            for (bucketPos = 0; bucketPos < MAP_SIZE; ++bucketPos) {

                struct Vec2i visPos = distances[(distance * MAP_SIZE) + bucketPos];

                if (visPos.x < 0 || visPos.y < 0 || visPos.x >= MAP_SIZE
                    || visPos.y >= MAP_SIZE) {
                    bucketPos = MAP_SIZE;
                    continue;
                }

                revealed[visPos.y][visPos.x] = TRUE;

                facesMask = MASK_LEFT | MASK_FRONT | MASK_RIGHT;

                switch (cameraDirection) {

                    case kNorth:
                        x = visPos.x;
                        z = visPos.y;
                        onTarget = (cursorX == x && cursorZ == z);
                        element = visibleElementsMap[ (z * MAP_SIZE) + x];

                        actorsSnapshotElement = mActors[z][x];
                        itemsSnapshotElement = mItems[z][x];
                        effectsSnapshotElement = mEffects[z][x];

                        position.mX =
                                mCamera.mX + intToFix(-2 * ((MAP_SIZE - 1) - x));
                        position.mY = mCamera.mY;
                        position.mZ =
                                mCamera.mZ + intToFix(2 * (MAP_SIZE) - (2 * z));

                        if (x > 0) {
                            facesMask |= (visibleElementsMap[(z * MAP_SIZE) + (x - 1)] != element) ?
                                         MASK_RIGHT :
                                         0;
                        }

                        /* remember, bounds - 1! */
                        if ((x < (MAP_SIZE - 1)) && (visibleElementsMap[(z * MAP_SIZE) + (x + 1)] == element) ) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((z < (MAP_SIZE - 1)) && (visibleElementsMap[((z + 1) * MAP_SIZE) + x] == element) ) {
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

                        element = visibleElementsMap[(z * MAP_SIZE) + x];
                        onTarget = (cursorX == x && cursorZ == z);
                        actorsSnapshotElement = mActors[z][x];
                        itemsSnapshotElement = mItems[z][x];
                        effectsSnapshotElement = mEffects[z][x];

                        position.mX = mCamera.mX + intToFix(-2 * x);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(2 + 2 * z);

                        /*						remember, bounds - 1!*/

                        if ((x > 0) && (visibleElementsMap[(z * MAP_SIZE) + (x - 1)] == element) ) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((x < (MAP_SIZE - 1)) && (visibleElementsMap[(z * MAP_SIZE) + (x + 1)] == element) ) {
                            facesMask &= ~MASK_RIGHT;
                        }
                        /*
                                        if (z < 0) {
                                            facesMask[1] = (visibleElementsMap[(z - 1)][x] !=
                                                            element);
                                        }
                */
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

                        element = visibleElementsMap[(x * MAP_SIZE) + z];
                        onTarget = (cursorX == z && cursorZ == x);
                        itemsSnapshotElement = mItems[x][z];
                        effectsSnapshotElement = mEffects[x][z];
                        actorsSnapshotElement = mActors[x][z];

                        position.mX = mCamera.mX + intToFix(-2 * x);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(2 - 2 * z);

                        /* remember, bounds - 1! */

                        if ((x > 0) && (visibleElementsMap[((x - 1) * MAP_SIZE) + z] == element)) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((x < (MAP_SIZE - 1)) && (visibleElementsMap[((x + 1) * MAP_SIZE) + z] == element)) {
                            facesMask &= ~MASK_RIGHT;
                        }

                        if ((z < (MAP_SIZE - 1)) && (visibleElementsMap[(x * MAP_SIZE) + (z + 1)] == element)) {
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

                        element = visibleElementsMap[(x * MAP_SIZE) + z];
                        onTarget = (cursorX == z && cursorZ == x);
                        actorsSnapshotElement = mActors[x][z];
                        itemsSnapshotElement = mItems[x][z];
                        effectsSnapshotElement = mEffects[x][z];

                        position.mX = mCamera.mX + intToFix(2 * x);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(2 * (z - MAP_SIZE + 1));


                        /* remember, bounds - 1! */
                        if ((x > 0) && (visibleElementsMap[((x - 1) * MAP_SIZE) + z] == element) ) {
                            facesMask &= ~MASK_RIGHT;
                        }

                        if ((x < (MAP_SIZE - 1)) && (visibleElementsMap[((x + 1) * MAP_SIZE) + z] == element)) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((z < (MAP_SIZE - 1)) && (visibleElementsMap[(x * MAP_SIZE) + (z - 1)] == element)) {
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

                if (tileProp->mFloorRepeatedTextureIndex != 0xFF
                    && tileProp->mFloorRepetitions > 0) {

                    switch (tileProp->mGeometryType) {
                        case kRightNearWall:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2)
                                       - intToFix(tileProp->mFloorRepetitions)),
                                      zero);

                            drawRightNear(
                                    tmp, intToFix(tileProp->mFloorRepetitions),
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex]
                                            ->rowMajor,
                                    facesMask, TRUE);

                            break;

                        case kLeftNearWall:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2)
                                       - intToFix(tileProp->mFloorRepetitions)),
                                      zero);

                            drawLeftNear(
                                    tmp, intToFix(tileProp->mFloorRepetitions),
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex]
                                            ->rowMajor, facesMask, TRUE);
                            break;

                        case kCube:
                        case kRampNorth:
                        case kRampEast:
                        default:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2)
                                       - intToFix(tileProp->mFloorRepetitions)),
                                      zero);

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

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mCeilingHeight * 2)
                                       + intToFix(tileProp->mCeilingRepetitions)),
                                      zero);

                            drawRightNear(
                                    tmp, intToFix(tileProp->mCeilingRepetitions),
                                    nativeTextures[tileProp->mCeilingRepeatedTextureIndex]
                                            ->rowMajor,
                                    facesMask, TRUE);
                            break;

                        case kLeftNearWall:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mCeilingHeight * 2)
                                       + intToFix(tileProp->mCeilingRepetitions)),
                                      zero);

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

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mCeilingHeight * 2)
                                       + intToFix(tileProp->mCeilingRepetitions)),
                                      zero);

                            drawColumnAt(
                                    tmp, intToFix(tileProp->mCeilingRepetitions),
                                    nativeTextures[tileProp->mCeilingRepeatedTextureIndex],
                                    facesMask, FALSE, TRUE);
                            break;
                    }
                }

                if (tileProp->mFloorTextureIndex != 0xFF) {

                    tmp.mX = position.mX;
                    tmp.mY = position.mY;
                    tmp.mZ = position.mZ;

                    addToVec3(&tmp, 0, (tileProp->mFloorHeight * 2), 0);



                    drawFloorAt(tmp, nativeTextures[tileProp->mFloorTextureIndex], cameraDirection);
                }

                if (tileProp->mCeilingTextureIndex != 0xFF) {

                    uint8_t newDirection = cameraDirection;

                    tmp.mX = position.mX;
                    tmp.mY = position.mY;
                    tmp.mZ = position.mZ;

                    addToVec3(&tmp, 0, (tileProp->mCeilingHeight * 2), 0);

                    if (cameraDirection == kNorth) {
                        newDirection = kSouth;
                    } if (cameraDirection == kSouth) {
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

                    if (((heightDiff * 2) - intToFix(integerPart)) >= halfOne) {
                        adjust = Div(halfOne, four);
                    }

                    switch (tileProp->mGeometryType) {
                        case kRightNearWall:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2) + heightDiff),
                                      zero);

                            drawRightNear(
                                    tmp, (heightDiff + Div(adjust, two)),
                                    nativeTextures[tileProp->mMainWallTextureIndex]->rowMajor,
                                    facesMask, tileProp->mRepeatMainTexture);
                            break;

                        case kLeftNearWall:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2) + heightDiff),
                                      zero);

                            drawLeftNear(
                                    tmp, (heightDiff + Div(adjust, two)),
                                    nativeTextures[tileProp->mMainWallTextureIndex]->rowMajor,
                                    facesMask, tileProp->mRepeatMainTexture);
                            break;
                            
                        case kRampNorth:
                        {
                            
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;
                            
                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;

                            
                            addToVec3(&tmp, 0, (tileProp->mFloorHeight * 2), 0);
                            addToVec3(&tmp2, 0, (tileProp->mCeilingHeight * 2), 0);
                            
                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection);
                        }
                            break;
                            
                        case kRampSouth:
                        {
                            
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;
                            
                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;
                            
                            
                            addToVec3(&tmp2, 0, (tileProp->mFloorHeight * 2), 0);
                            addToVec3(&tmp, 0, (tileProp->mCeilingHeight * 2), 0);
                            
                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection);
                        }
                            break;
                            
                        case kRampEast:
                        {
                            
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;
                            
                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;
                            
                            
                            addToVec3(&tmp2, 0, (tileProp->mFloorHeight * 2), 0);
                            addToVec3(&tmp, 0, (tileProp->mCeilingHeight * 2), 0);
                            
                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], (cameraDirection + 1) & 3 );
                        }
                            break;
                        case kRampWest:
                        {
                            
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;
                            
                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;
                            
                            
                            addToVec3(&tmp2, 0, (tileProp->mFloorHeight * 2), 0);
                            addToVec3(&tmp, 0, (tileProp->mCeilingHeight * 2), 0);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], (cameraDirection + 3) & 3);
                        }
                            break;
                        case kCube:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2) + heightDiff),
                                      zero);

                            drawColumnAt(tmp, (heightDiff + Div(adjust, two)),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                        default:
                            break;
                    }
                }


                if (itemsSnapshotElement != 0xFF) {
                    tmp.mX = position.mX;
                    tmp.mY = position.mY;
                    tmp.mZ = position.mZ;
                    
                    addToVec3(&tmp, 0, (tileProp->mFloorHeight * 2) + one, 0);
                    
                    drawBillboardAt(tmp, itemSprites[itemsSnapshotElement]->rotations[0], one, 32);
                }
            }
        }

        clippingY1 = 200;

        drawRect(256, 0, 64, 128, 0);

        fill(0, 0, 320, 8, 0, FALSE);
        sprintf(&buffer[0], "Health: %d%", getPlayerHealth());
        drawTextAt(2, 1, &buffer[0], 255);
        fill(256, 8, 320 - 256, 160 - 8, 255, FALSE);
        drawTextAt(34, 1, "Items", 255);
        
        
        int line = 0;
        struct ObjectNode* head = getPlayerItems();
        
        while(head != NULL) {
            if (head->item != NULL ) {
                char buffer[255];
                sprintf( &buffer[0], "%c%c%s", ( line == currentSelectedItem ? '>' : ' ' ), (head->item->active ? '*' : ' '), head->item->description);
                buffer[7] = 0;
                drawTextAt(34, 2 + line, &buffer[0], 0);
                
                if (line == currentSelectedItem ) {
                    drawFrontWall(intToFix(0), intToFix(199 - 32), intToFix(31), intToFix(199), itemSprites[head->item->index]->rotations[0], one, 0, 0, 32);
                }

                ++line;
            }
            head = head->next;
        }

        
        if (focusItemName != NULL ) {
            drawTextAt(2, 20, focusItemName, 255);
        }
        
        if (messageLogBufferCoolDown > 0 ) {
            drawTextAt(2, 23, &messageLogBuffer[0], 255);
        }

        
        
    }
    /*
    uint8_t uvCoords[6];
    uvCoords[0] = 0;
    uvCoords[1] = 0;
    uvCoords[2] = 0;
    uvCoords[3] = 255;
    uvCoords[4] = 255;
    uvCoords[5] = 255;
    
    drawTexturedTriangle( &coords[0], &uvCoords[0], nativeTextures[17] );
    fill( coords[0] - 2, coords[1] - 2, 4, 4, 0, 1);
    fill( coords[2] - 2, coords[3] - 2, 4, 4, 30, 1);
    fill( coords[4] - 2, coords[5] - 2, 4, 4, 255, 1);
     */
}
