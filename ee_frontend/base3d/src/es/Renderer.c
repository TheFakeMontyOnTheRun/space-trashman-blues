#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

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
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "UI.h"
#include "Engine.h"

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

struct Mesh mesh;
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
struct MapWithCharKey tileProperties;
struct Vec2i cameraPosition;
enum ECommand mBufferedCommand = kCommandNone;
struct Texture *itemSprites[TOTAL_ITEMS];
int turnTarget = 0;
int turnStep = 0;
FixP_t xCameraOffset;
FixP_t yCameraOffset;
FixP_t zCameraOffset;
struct Bitmap *mapTopLevel;
char messageLogBuffer[256];

int messageLogBufferCoolDown = 0;

extern unsigned int vs, fs, program;

typedef float t_mat4x4[16];

t_mat4x4 projection_matrix;

extern unsigned int aPositionAttributeLocation;
extern unsigned int aTexCoordAttributeLocation;
extern unsigned int uProjectionViewUniformLocation;
extern unsigned int sTextureUniformLocation;
extern unsigned int uModUniformLocation;
extern unsigned int uFadeUniformLocation;



const float planeXYVertices[] = {
        -1.0f, 1.0f, 0.0f, 0.0f, .0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
};

const float planeYZVertices[] = {
        0.0f, 1.0f, -1.0f, 0.0f, .0f,
        0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, -1.0f, -1.0f, 0.0f, 1.0f,
};


const float cornerLeftFarVertices[] = {
        -1.0f, 1.0f, -1.0f, 0.0f, .0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
};

const float cornerLeftNearVertices[] = {
        -1.0f, 1.0f, 1.0f, 0.0f, .0f,
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
};


const float floorVertices[] = {
        -1.0f, 0.0f, -1.0f, 0.0f, .0f,
        1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
};

const unsigned short planeXYIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short planeYZIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short cornerLeftFarIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short cornerLeftNearIndices[] = {
        0, 1, 2,
        0, 2, 3
};

const unsigned short floorIndices[] = {
        0, 1, 2,
        0, 2, 3
};

struct VBORegister planeXYVBO, leftFarVBO, leftNearVBO, floorVBO, planeYZVBO;

static inline void mat4x4_ortho( t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar ) {
    #define T(a, b) (a * 4 + b)

    out[T(0,0)] = 2.0f / (right - left);
    out[T(0,1)] = 0.0f;
    out[T(0,2)] = 0.0f;
    out[T(0,3)] = 0.0f;

    out[T(1,1)] = 2.0f / (top - bottom);
    out[T(1,0)] = 0.0f;
    out[T(1,2)] = 0.0f;
    out[T(1,3)] = 0.0f;

    out[T(2,2)] = -2.0f / (zfar - znear);
    out[T(2,0)] = 0.0f;
    out[T(2,1)] = 0.0f;
    out[T(2,3)] = 0.0f;

    out[T(3,0)] = -(right + left) / (right - left);
    out[T(3,1)] = -(top + bottom) / (top - bottom);
    out[T(3,2)] = -(zfar + znear) / (zfar - znear);
    out[T(3,3)] = 1.0f;

    #undef T
}


static inline void mat4x4_perspective( t_mat4x4 out, float fov, float ratio, float znear, float zfar ) {
    out[0] = 0.463027f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = 0.0f;
    out[4] = 0.0f;
    out[5] = 0.61737;
    out[6] = 0.0f;
    out[7] = 0.0f;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = -1.002f;
    out[11] = -1.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = -0.2002f;
    out[15] = 0.0f;
}

struct VBORegister submitVBO(float *data, int vertices,
                             unsigned short *indexData,
                             unsigned int indices) {

    unsigned int dataIndex;
    unsigned int indicesIndex;

    glGenBuffers(1, &dataIndex);
    glBindBuffer(GL_ARRAY_BUFFER, dataIndex);
    glBufferData(GL_ARRAY_BUFFER, vertices * sizeof(float) * 5, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &indicesIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices * sizeof(GLushort), indexData,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    struct VBORegister toReturn;
    toReturn.dataIndex = dataIndex;
    toReturn.indicesIndex = indicesIndex;
    toReturn.indices = indices;

    return toReturn;
}

void printMessageTo3DView(const char *message);

enum EVisibility visMap[MAP_SIZE * MAP_SIZE];
struct Vec2i distances[2 * MAP_SIZE * MAP_SIZE];


void drawTriangle(const struct Vec3 pos1,
                  const struct Vec2i uv1,
                  const struct Vec3 pos2,
                  const struct Vec2i uv2,
                  const struct Vec3 pos3,
                  const struct Vec2i uv3,
                  const struct Texture *texture);

uint32_t getPaletteEntry(const uint32_t origin) {
    return (0x80 << 24) + (origin & 0x00FFFFFF);
}

void enter2D(void) {
  mat4x4_ortho( projection_matrix, 0.0f, (float)XRES_FRAMEBUFFER, (float)YRES_FRAMEBUFFER, 0.0f, 0.1f, 100.0f );
  glUniformMatrix4fv( uProjectionViewUniformLocation, 1, GL_FALSE, projection_matrix );
  glDisable( GL_DEPTH_TEST );
}

void initGL() {
    /* tmp */
    memFill(&nativeTextures[0], 0, sizeof(struct Texture) * TOTAL_TEXTURES);


    planeXYVBO = submitVBO((float *) planeXYVertices, 4,
                                           (unsigned short *) planeXYIndices, 6);

    planeYZVBO = submitVBO((float *) planeYZVertices, 4,
                                           (unsigned short *) planeYZIndices, 6);
	
    leftFarVBO = submitVBO((float *) cornerLeftFarVertices, 4,
                                         (unsigned short *) cornerLeftFarIndices, 6);
    leftNearVBO = submitVBO((float *) cornerLeftNearVertices, 4,
                                          (unsigned short *) cornerLeftNearIndices, 6);
    floorVBO = submitVBO((float *) floorVertices, 4,
                                       (unsigned short *) floorIndices, 6);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //    glFrontFace(GL_CW);
    glDepthMask(1);
}

void clearRenderer() {
}

void bindTexture(struct Bitmap *bitmap) {
    glBindTexture(GL_TEXTURE_2D, bitmap->uploadId);
}

int submitBitmapToGPU(struct Bitmap *bitmap) {
    // Texture object handle
    unsigned int textureId = 0;

    //Generate texture storage
    glGenTextures(1, &textureId);

    //specify what we want for that texture
    glBindTexture(GL_TEXTURE_2D, textureId);

    //upload the data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->width, bitmap->height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bitmap->data);

    // Set the filtering mode - surprisingly, this is needed.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return bitmap->uploadId = textureId;
}


void startFrameGL(int x, int y, int width, int height) {
    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = FALSE;

    float fade[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUniform4fv(uFadeUniformLocation, 1, &fade[0]);
    glDepthFunc(GL_LEQUAL);
    glUniform4f(uModUniformLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    glUniform1i(aTexCoordAttributeLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    enter2D();
}

void endFrameGL() {
}

void enter3D(void) {
    float _leanX = 0.0f;
    float _leanY = 0.0f;

    if (leanX > 127) {
        _leanX = -0.25f * ((leanX - 127) / 128.0f);
    }

    if (leanX < 127) {
        _leanX = 0.25f * ((128 - leanX) / 127.0f);
    }

    if (leanY > 127) {
        _leanY = -0.25f * ((leanY - 127) / 128.0f);
    }

    if (leanY < 127) {
        _leanY = 0.25f * ((128 - leanY) / 127.0f);
    }

    mat4x4_perspective( projection_matrix, 45.0f, (float)XRES_FRAMEBUFFER / (float)YRES_FRAMEBUFFER, 0.1f, 100.0f );
    glUniformMatrix4fv( uProjectionViewUniformLocation, 1, GL_FALSE, projection_matrix );
    glEnable(GL_DEPTH_TEST);
}

void printMessageTo3DView(const char *message) {
    strcpy(&messageLogBuffer[0], message);
    messageLogBufferCoolDown = 5000;
}

void loadTileProperties(const uint8_t levelNumber) {
    char buffer[64];
    int c;

    setLoggerDelegate(printMessageTo3DView);

    clearMap(&tileProperties);
    clearMap(&occluders);
    clearMap(&colliders);

    sprintf(buffer, "props%d.bin", levelNumber);
    struct StaticBuffer data = loadBinaryFileFromPath(buffer);
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
    char buffer[256];
    struct StaticBuffer data;
    char tilesFilename[64];
    char *head;
    char *end;
    char *nameStart;

    sprintf(tilesFilename, "tiles%d.lst", levelNumber);
    data = loadBinaryFileFromPath(tilesFilename);

    char *tmp = (char *) allocMem(data.size, GENERAL_MEMORY, 1);
    head = tmp;
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

    disposeMem(tmp);

    /* tmp */
    playerHeight = -intToFix(1);

    loadMesh(&mesh, "fighter.mdl");
}

void updateCursorForRenderer(const int x, const int z) {
    needsToRedrawVisibleMeshes = TRUE;
    visibilityCached = FALSE;
    cursorX = x;
    cursorZ = z;
}

void renderRoomTransition() {
    struct Vec3 center;

    if (!enableSmoothMovement) {
        currentPresentationState = kWaitingForInput;
        zCameraOffset = xCameraOffset = yCameraOffset = 0;
        needsToRedrawHUD = TRUE;
        return;
    }

    xCameraOffset = yCameraOffset = 0;

    enter3D();

    center.mY = 0;
    center.mZ = intToFix(3);
    center.mX = -intToFix(3);
    drawColumnAt(center, intToFix(2), nativeTextures[1], MASK_LEFT, 0, 1);

    center.mY = 0;
    center.mX = intToFix(3);
    drawColumnAt(center, intToFix(2), nativeTextures[1], MASK_RIGHT, 0, 1);

    center.mZ = intToFix(2);
    center.mX = -intToFix(1);
    center.mY = intToFix(4) - zCameraOffset;
    drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);

    center.mX = intToFix(1);
    drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);

    center.mZ = intToFix(2);
    center.mY = intToFix(3) - zCameraOffset;

    center.mX = -intToFix(1);
    drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);

    center.mX = intToFix(1);
    drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);


    center.mY = intToFix(5) - zCameraOffset;

    center.mX = -intToFix(1);
    drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);

    center.mX = intToFix(1);
    drawBillboardAt(center, nativeTextures[0], intToFix(1), 32);

    enter2D();

    drawTextAtWithMargin(((XRES / 8) / 2) - (thisMissionNameLen / 2), 1, XRES, thisMissionName,
                         getPaletteEntry(0xFFFFFFFF));

    zCameraOffset -= Div(intToFix(1), intToFix(32));

    if (zCameraOffset == 0) {
        currentPresentationState = kWaitingForInput;
        needsToRedrawVisibleMeshes = TRUE;
        gameTicks = 0;
        needsToRedrawHUD = TRUE;
    }
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
    static FixP_t zero = 0;
    FixP_t two = intToFix(2);
    FixP_t four = intToFix(4);
    FixP_t one = intToFix(1);
    const FixP_t halfOne = Div(one, two);
    FixP_t standardHeight = Div(intToFix(180), intToFix(100));

    if (!hasSnapshot) {
        return;
    }

    if (playerHeight < playerHeightTarget) {
        playerHeight += playerHeightChangeRate;
    }

    if (needsToRedrawVisibleMeshes) {
        char buffer[64];
        char directions[4] = {'N', 'E', 'S', 'W'};
        struct Vec3 tmp;
        struct CTile3DProperties *tileProp;
        FixP_t heightDiff;
        uint8_t lastElement = 0xFF;
        uint8_t itemsSnapshotElement = 0xFF;
        uint8_t element = 0;
        struct Vec3 position;
        FixP_t tileHeight = 0;
        int16_t x, y, z;
        int distance;
        FixP_t cameraHeight;
        int c;
        uint8_t facesMask;

        needsToRedrawVisibleMeshes = FALSE;

        element = LEVEL_MAP(cameraPosition.x, cameraPosition.y);

        tileProp = ((struct CTile3DProperties *) getFromMap(&tileProperties,
                                                            element));

        if (tileProp) {
            tileHeight = tileProp->mFloorHeight;
        }

        cameraHeight = -2 * tileHeight;

        mCamera.mY = cameraHeight - standardHeight;

        enter3D();

        for (distance = 0; distance < (MAP_SIZE + MAP_SIZE); ++distance) {
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

                        element = LEVEL_MAP(z, x);
                        itemsSnapshotElement = ITEMS_IN_MAP(z, x);

                        position.mX = mCamera.mX + intToFix(-2 * x + 1) - intToFix(1);
                        position.mY = mCamera.mY;
                        position.mZ = mCamera.mZ + intToFix(1 - 2 * z);

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
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex],
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
                                    nativeTextures[tileProp->mFloorRepeatedTextureIndex], facesMask, TRUE);
                            break;

                        case kCube:
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
                                    nativeTextures[tileProp->mCeilingRepeatedTextureIndex],
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
                                    nativeTextures[tileProp->mCeilingRepeatedTextureIndex],
                                    facesMask, TRUE);
                            break;

                        case kCube:
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

                    drawCeilingAt(
                            tmp, nativeTextures[tileProp->mCeilingTextureIndex], newDirection);
                }

                if (tileProp->mGeometryType != kNoGeometry
                    && tileProp->mMainWallTextureIndex != 0xFF) {
                    struct Vec3 tmp2;
                    int integerPart = fixToInt(tileProp->mCeilingHeight)
                                      - fixToInt(tileProp->mFloorHeight);

                    FixP_t adjust = 0;

                    if (((heightDiff * 2) - intToFix(integerPart)) >= halfOne) {
                        adjust = Div(halfOne, four);
                    }

                    switch (tileProp->mGeometryType) {
                        case kWallNorth:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2) + heightDiff),
                                      zero);

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

                            drawColumnAt(tmp, (heightDiff + Div(adjust, two)),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                            break;
                        case kWallWest:


                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2) + heightDiff),
                                      zero);

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

                            drawColumnAt(tmp, (heightDiff + Div(adjust, two)),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                            break;

                        case kWallCorner:


                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2) + heightDiff),
                                      zero);

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

                            drawColumnAt(tmp, (heightDiff + Div(adjust, two)),
                                         nativeTextures[tileProp->mMainWallTextureIndex],
                                         facesMask, tileProp->mNeedsAlphaTest,
                                         tileProp->mRepeatMainTexture);
                            break;

                        case kRightNearWall:

                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            addToVec3(&tmp, zero,
                                      ((tileProp->mFloorHeight * 2) + heightDiff),
                                      zero);

                            drawRightNear(
                                    tmp, (heightDiff + Div(adjust, two)),
                                    nativeTextures[tileProp->mMainWallTextureIndex],
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
                                    nativeTextures[tileProp->mMainWallTextureIndex],
                                    facesMask, tileProp->mRepeatMainTexture);
                            break;
                        case kRampNorth: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;

                            addToVec3(&tmp, 0, (tileProp->mFloorHeight) * 2, 0);
                            addToVec3(&tmp2, 0, (tileProp->mCeilingHeight * 2), 0);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection,
                                       flipTextureVertical);
                        }
                            break;

                        case kRampSouth: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;

                            addToVec3(&tmp2, 0, (tileProp->mFloorHeight * 2), 0);
                            addToVec3(&tmp, 0, (tileProp->mCeilingHeight * 2), 0);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection,
                                       flipTextureVertical);
                        }
                            break;

                        case kRampEast: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;

                            addToVec3(&tmp2, 0, (tileProp->mFloorHeight * 2), 0);
                            addToVec3(&tmp, 0, (tileProp->mCeilingHeight * 2), 0);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex],
                                       (cameraDirection + 1) & 3, flipTextureVertical);
                        }
                            break;
                        case kRampWest: {
                            uint8_t flipTextureVertical = 0;
                            tmp.mX = position.mX;
                            tmp.mY = position.mY;
                            tmp.mZ = position.mZ;

                            tmp2.mX = position.mX;
                            tmp2.mY = position.mY;
                            tmp2.mZ = position.mZ;


                            addToVec3(&tmp2, 0, (tileProp->mFloorHeight * 2), 0);
                            addToVec3(&tmp, 0, (tileProp->mCeilingHeight * 2), 0);

                            drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex],
                                       (cameraDirection + 3) & 3, flipTextureVertical);
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

                    // lazy loading the item sprites
                    if (itemSprites[itemsSnapshotElement] == NULL) {
                        char buffer[64];
                        sprintf(&buffer[0], "%s.img", getItem(itemsSnapshotElement)->name);
                        itemSprites[itemsSnapshotElement] = makeTextureFrom(&buffer[0]);
                    }

                    drawBillboardAt(tmp, itemSprites[itemsSnapshotElement], one, 32);
                }

            }
        }

        enter2D();

        if (focusItemName != NULL) {
            size_t len = strlen(focusItemName);
            int lines = 1 + (len / 27);
            fillRect(0, YRES - (8 * lines), XRES, lines * 8, 0, 1);
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

            fillRect(0, 0, 216, lines * 8, 0, 1);

            drawTextAt(1, 1, &messageLogBuffer[0], 255);
        }

        redrawHUD();
    }
}
