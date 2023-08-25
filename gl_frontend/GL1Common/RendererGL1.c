#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#ifdef N64
#include <libdragon.h>
#include <GL/gl_integration.h>
#endif

#ifndef NDS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION

#include <OpenGL/gl.h>

#else
#include <GL/gl.h>
#endif
#else
#include <nds.h>
#include <malloc.h>
#include <stdio.h>
#include <nds/arm9/image.h>
#include <nds/arm9/trig_lut.h>
#endif

#include "Core.h"
#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "Vec.h"
#include "CActor.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "Globals.h"
#include "LoadBitmap.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "UI.h"
#include "Engine.h"

#define STANDARD_HEIGHT (Div(intToFix(180), intToFix(100)))

extern int leanX;
extern int leanY;
int visibilityCached = FALSE;
int needsToRedrawVisibleMeshes = TRUE;
uint8_t texturesUsed = 0;
int hasSnapshot = FALSE;
FixP_t playerHeightChangeRate = 0;
FixP_t playerHeightTarget = -intToFix(1);
int cursorX = -1;
int cursorZ = -1;
uint8_t enableSmoothMovement;
struct MapWithCharKey occluders;
struct MapWithCharKey colliders;
enum EDirection cameraDirection;
struct Vec3 mCamera;
long gameTicks = 0;
uint8_t linesOfSight[MAP_SIZE][MAP_SIZE];
uint8_t revealed[MAP_SIZE][MAP_SIZE];
struct MapWithCharKey tileProperties;
struct Vec2i cameraPosition;
enum ECommand mBufferedCommand = kCommandNone;
struct Texture *itemSprites[TOTAL_ITEMS];
int turnTarget = 0;
int turnStep = 0;
FixP_t xCameraOffset;
FixP_t yCameraOffset;
FixP_t zCameraOffset;
struct Bitmap *mapTopLevel[8];
char messageLogBuffer[256];

int messageLogBufferCoolDown = 0;

void printMessageTo3DView(const char *message);

#ifdef N64
static surface_t zbuffer;
#endif


enum EVisibility visMap[MAP_SIZE * MAP_SIZE];
struct Vec2i distances[2 * MAP_SIZE * MAP_SIZE];

uint32_t getPaletteEntry(const uint32_t origin) {
    return origin;
}


void enter2D(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 3.2, 2, 0, -0.3, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#ifndef NDS
    glDisable(GL_DEPTH_TEST);
#endif
}

void initGL(void) {

#ifdef N64
    zbuffer = surface_alloc(FMT_RGBA16, display_get_width(), display_get_height());
#endif

    glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping ( NEW )
    glClearColor(0, 0, 0, 1);                   // Black Background
    glDisable(GL_FOG);
#ifndef NDS
    glClearDepth(1);                         // Depth Buffer Setup
    glShadeModel(GL_SMOOTH);                        // Enable Smooth Shading
    glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
    glAlphaFunc(GL_GREATER, 0);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_CULL_FACE);
#else
    glClearDepth(GL_MAX_DEPTH);
#endif
}

void startFrameGL(int width, int height) {
#ifdef N64
    surface_t *disp = display_get();

    rdpq_attach(disp, &zbuffer);

    gl_context_begin();
#endif


    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);                   // Black Background

#ifndef NDS
    glLineWidth(width / 240.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
#else
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
    glClearDepth(GL_MAX_DEPTH);
#endif

    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = FALSE;
    enter2D();
}

void endFrameGL(void) {
#ifndef NDS
    int error;
    glFinish();

    error = glGetError();

    if (error) {
        printf("glError: %d\n", error);
    }
#else
    glFlush(0);
#endif

#ifdef N64
    gl_context_end();

    rdpq_detach_show();
#endif
}


void setPerspective(float fovy,
                    float aspect,
                    float zNear,
                    float zFar) {

    float aspect_ratio = aspect;
    float near_plane = zNear;
    float far_plane = zFar;

    glFrustum(-near_plane * aspect_ratio, near_plane * aspect_ratio, -near_plane, near_plane, near_plane, far_plane);
}

void enter3D(void) {
#ifndef NDS
    GLfloat fogColor[4] = {0.0, 0.0, 0.0, 1.0};
#else
    int i;
#endif
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    setPerspective(45, 240.0f / 200.0f, 1, 1024);
    glScalef(1, 1.25f, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(((float) leanX), 0.0f, 1.0f, 0.0f);
    glRotatef(((float) leanY), 1.0f, 0.0f, 0.0f);
    glEnable(GL_FOG);

#ifndef NDS
    glEnable(GL_DEPTH_TEST);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 16);
    glFogf(GL_FOG_END, 48);
    glFogf(GL_FOG_DENSITY, 0.75);
#else
    glFogShift(2);
    glFogColor(0,0,0,0);

    for(i=0;i<32;i++) {
        glFogDensity(i,i*4);
    }

    glFogDensity(31,127);
    glFogOffset(0x6000);

    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FOG);
#endif

    glColor3f(1, 1, 1);
}

void printMessageTo3DView(const char *message) {
    strcpy(&messageLogBuffer[0], message);
    messageLogBufferCoolDown = 5000;
}

void loadTileProperties(const uint8_t levelNumber) {
    char buffer[64];
    int c;
    struct StaticBuffer data;

    setLoggerDelegate(printMessageTo3DView);

    clearMap(&tileProperties);
    clearMap(&occluders);
    clearMap(&colliders);

    sprintf(buffer, "props%d.bin", levelNumber);
    data = loadBinaryFileFromPath(buffer);

    for (c = 0; c < 256; ++c) {
        disposeMem((void *) getFromMap(&tileProperties, c));
    }

    loadPropertyList(&buffer[0], &tileProperties);

    for (c = 0; c < 256; ++c) {
        struct CTile3DProperties *prop =
                (struct CTile3DProperties *) getFromMap(&tileProperties, c);

        if (prop && prop->mBlockVisibility) {
            setInMap(&occluders, c, &occluders);
        } else {
            setInMap(&occluders, c, NULL);
        }
    }

    disposeDiskBuffer(data);
}

void loadTexturesForLevel(const uint8_t levelNumber) {

    struct StaticBuffer data;
    char tilesFilename[64];
    char *head;
    char *end;
    char *nameStart;

    sprintf(tilesFilename, "tiles%d.lst", levelNumber);
    data = loadBinaryFileFromPath(tilesFilename);
    char *buffer = (char *) allocMem(data.size, GENERAL_MEMORY, 1);
    head = buffer;
    memCopyToFrom(head, data.data, data.size);
    end = head + data.size;
    disposeDiskBuffer(data);
    nameStart = head;

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
    needsToRedrawVisibleMeshes = TRUE;
    visibilityCached = FALSE;
    cursorX = x;
    cursorZ = z;
}

void drawMap(const struct CActor *current) {

    int8_t z, x;
    const struct Vec2i mapCamera = current->position;
    cameraDirection = current->rotation;
    hasSnapshot = TRUE;

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

    if ((cameraPosition.x + cameraPosition.y) & 1) {
        walkingBias = WALKING_BIAS;
    } else {
        walkingBias = 0;
    }

    castVisibility(cameraDirection, visMap, cameraPosition,
                   distances, TRUE, &occluders);

    ++gameTicks;
}

enum ECommand getInput() {
    const enum ECommand toReturn = mBufferedCommand;
    mBufferedCommand = kCommandNone;
    return toReturn;
}

void render(const long ms) {

    if (!hasSnapshot) {
        return;
    }

    if (playerHeight < playerHeightTarget) {
        playerHeight += playerHeightChangeRate;
    }

    if (needsToRedrawVisibleMeshes) {
        struct Vec3 tmp;
        struct Vec3 tmp2;
        struct CTile3DProperties *tileProp;
        FixP_t heightDiff;
        uint8_t lastElement = 0xFF;
        uint8_t itemsSnapshotElement = 0xFF;
        uint8_t element = 0;
        struct Vec3 position;
        FixP_t tileHeight = 0;
        int16_t x, z;
        int distance;
        FixP_t cameraHeight;
        uint8_t facesMask;

        needsToRedrawVisibleMeshes = FALSE;

        element = LEVEL_MAP(cameraPosition.x, cameraPosition.y);

        tileProp = ((struct CTile3DProperties *) getFromMap(&tileProperties,
                                                            element));

        if (tileProp) {
            tileHeight = tileProp->mFloorHeight;
        }

        cameraHeight = -2 * tileHeight;

        mCamera.mY = cameraHeight - STANDARD_HEIGHT;

        enter3D();

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
                        element = LEVEL_MAP(x, z);

                        itemsSnapshotElement = ITEMS_IN_MAP(x, z);

                        position.mX =
                                mCamera.mX + intToFix(-2 * ((MAP_SIZE - 1) - x));
                        position.mY = mCamera.mY;
                        position.mZ =
                                mCamera.mZ + intToFix(2 * (MAP_SIZE - z) - 1);

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
                        position.mZ = mCamera.mZ + intToFix(2 * z) + intToFix(1);

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

                        element = LEVEL_MAP(z, x );
                        itemsSnapshotElement = ITEMS_IN_MAP(z, x);

                        position.mX = mCamera.mX + intToFix(-2 * x + 1) - intToFix(1);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(1 - 2 * z);

                        /* remember, bounds - 1! */

                        if ((x > 0) && (LEVEL_MAP(z , x - 1) == element)) {
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

                        element = LEVEL_MAP(z, x);
                        itemsSnapshotElement = ITEMS_IN_MAP(z, x);

                        position.mX = mCamera.mX + intToFix(2 * x - 1) + intToFix(1);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(2 * (z - MAP_SIZE) + 1);


                        /* remember, bounds - 1! */
                        if ((x > 0) && (LEVEL_MAP(z, x - 1) == element)) {
                            facesMask &= ~MASK_RIGHT;
                        }

                        if ((x < (MAP_SIZE - 1)) && (LEVEL_MAP(z, x + 1) == element)) {
                            facesMask &= ~MASK_LEFT;
                        }

                        if ((z < (MAP_SIZE - 1)) && (LEVEL_MAP(z - 1, x ) == element)) {
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
                        assert(FALSE);
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

                FixP_t zPos = zCameraOffset + position.mZ;
                FixP_t xPos = xCameraOffset + position.mX;
                glTranslatef(fixToFloat(xPos), 0, -fixToFloat(zPos));
                tmp.mX = tmp2.mX = position.mX;
                tmp.mZ = tmp2.mZ = position.mZ;

                if (tileProp->mFloorRepeatedTextureIndex != 0xFF
                    && tileProp->mFloorRepetitions > 0) {

                    switch (tileProp->mGeometryType) {
                        case kRightNearWall:

                            tmp.mY = position.mY +
                                     ((tileProp->mFloorHeight * 2)
                                      - intToFix(tileProp->mFloorRepetitions));
                            drawRightNear(
                                    tmp, intToFix(tileProp->mFloorRepetitions),
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex],
                                    facesMask, TRUE);

                            break;

                        case kLeftNearWall:
                            tmp.mY = position.mY +
                                     ((tileProp->mFloorHeight * 2) - intToFix(tileProp->mFloorRepetitions));

                            drawLeftNear(
                                    tmp, intToFix(tileProp->mFloorRepetitions),
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex], facesMask, TRUE);
                            break;

                        case kCube:
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
                                     ((tileProp->mCeilingHeight * 2) + intToFix(tileProp->mCeilingRepetitions)),
                                    drawRightNear(
                                            tmp, intToFix(tileProp->mCeilingRepetitions),
                                            nativeTextures[tileProp->mCeilingRepeatedTextureIndex],
                                            facesMask, TRUE);
                            break;

                        case kLeftNearWall:
                            tmp.mY = position.mY +
                                     ((tileProp->mCeilingHeight * 2) + intToFix(tileProp->mCeilingRepetitions)),
                                    drawLeftNear(
                                            tmp, intToFix(tileProp->mCeilingRepetitions),
                                            nativeTextures[tileProp->mCeilingRepeatedTextureIndex],
                                            facesMask, TRUE);
                            break;

                        case kCube:
                        default:
                            tmp.mY = position.mY +
                                     ((tileProp->mCeilingHeight * 2) + intToFix(tileProp->mCeilingRepetitions)),

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
                                    nativeTextures[tileProp->mMainWallTextureIndex],
                                    facesMask, tileProp->mRepeatMainTexture);
                            break;

                        case kLeftNearWall:
                            tmp.mY = position.mY + ((tileProp->mFloorHeight * 2) + heightDiff);
                            drawLeftNear(
                                    tmp, (heightDiff + adjust),
                                    nativeTextures[tileProp->mMainWallTextureIndex],
                                    facesMask, tileProp->mRepeatMainTexture);
                            break;
                        case kRampNorth: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mFloorHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mCeilingHeight * 2);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection,
                                       flipTextureVertical);
                        }
                            break;

                        case kRampSouth: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mCeilingHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mFloorHeight * 2);
                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection,
                                       flipTextureVertical);
                        }
                            break;

                        case kRampEast: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mCeilingHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mFloorHeight * 2);
                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex],
                                       (cameraDirection + 1) & 3, flipTextureVertical);
                        }
                            break;
                        case kRampWest: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mY = position.mY + (tileProp->mCeilingHeight * 2);
                            tmp2.mY = position.mY + (tileProp->mFloorHeight * 2);
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

                    // lazy loading the item sprites
                    // we can't preload it because...reasons on the NDS
                    // perhaps some state machine issue? IDK. Placing this here works better for the NDS.
                    if (itemSprites[itemsSnapshotElement] == NULL) {
                        char buffer[64];
                        sprintf(&buffer[0], "%s.img", getItem(itemsSnapshotElement)->name);
                        itemSprites[itemsSnapshotElement] = makeTextureFrom(&buffer[0]);
                    }

                    glRotatef(((float) -leanX), 0.0f, 1.0f, 0.0f);
                    drawBillboardAt(tmp, itemSprites[itemsSnapshotElement], intToFix(1), 32);
                    glRotatef(((float) leanX), 0.0f, 1.0f, 0.0f);

                }
                glTranslatef(-fixToFloat(xPos), 0, fixToFloat(zPos));
            }
        }
#ifdef NDS
        swiWaitForVBlank();
#endif
        enter2D();

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
            int len = strlen(&messageLogBuffer[0]);
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

            drawTextAt(1, 1, &messageLogBuffer[0], 255);
        }

        redrawHUD();
    }
}
