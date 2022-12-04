#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
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

char mTurnBuffer = kCommandNone;
int currentTarget;
int visibilityCached = FALSE;
int needsToRedrawVisibleMeshes = TRUE;
uint8_t texturesUsed = 0;
int gunSpeedY = 0;
int hasSnapshot = FALSE;
int gunSpeedX = 0;
FixP_t playerHeightChangeRate = 0;
FixP_t playerHeightTarget = 0;
int cursorX = -1;
int cursorZ = -1;
uint8_t enableSmoothMovement;
struct Vec2i gunPosition;
struct Vec2i gunTargetPosition;
uint8_t grabbingDisk = FALSE;
uint8_t covered = FALSE;
struct MapWithCharKey occluders;
struct MapWithCharKey colliders;
struct MapWithCharKey enemySightBlockers;
enum EDirection cameraDirection;
int playerHealth = 0;
struct Vec3 mCamera;
long gameTicks = 0;
int distanceForPenumbra = 16;
int distanceForDarkness = 32;
int playerAmmo = 0;
uint8_t linesOfSight[MAP_SIZE][MAP_SIZE];
uint8_t revealed[MAP_SIZE][MAP_SIZE];
struct Bitmap *backdrop = NULL;
struct MapWithCharKey tileProperties;
struct Vec2i cameraPosition;
uint8_t mItems[MAP_SIZE][MAP_SIZE];
uint8_t shouldShowDamageHighlight = FALSE;
uint8_t shouldShowDetectedHighlight = FALSE;
int highlightDisplayTime = 0;
enum ECommand mBufferedCommand = kCommandNone;
struct Texture *itemSprites[TOTAL_ITEMS];
int turnTarget = 0;
int turnStep = 0;
FixP_t xCameraOffset;
FixP_t yCameraOffset;
FixP_t zCameraOffset;
struct Bitmap *mapTopLevel;

enum EVisibility visMap[MAP_SIZE * MAP_SIZE];
struct Vec2i distances[2 * MAP_SIZE * MAP_SIZE];

uint32_t getPaletteEntry(const uint32_t origin) {
    return origin;
}

void enter3D(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    
    glEnable(GL_DEPTH_TEST);
    glColor3f(1,1,1);
    
    
    GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};
    glFogi(GL_FOG_MODE, GL_EXP);        // Fog Mode
    glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
    glFogf(GL_FOG_DENSITY, 0.05f);              // How Dense Will The Fog Be
    glHint(GL_FOG_HINT, GL_FASTEST);          // Fog Hint Value
    glEnable(GL_FOG);                   // Enables GL_FOG
}

void enter2D(void) {
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     glOrtho(0, 320, 200, 0, -100, 100);

     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();

     glDisable(GL_DEPTH_TEST);
     glDisable(GL_FOG);
}


void loadTileProperties(const uint8_t levelNumber) {
    clearMap(&tileProperties);
    clearMap(&occluders);
    clearMap(&colliders);

    gunTargetPosition.x = gunPosition.x;

    char buffer[64];

    sprintf(buffer, "props%d.bin", levelNumber);

    struct StaticBuffer data = loadBinaryFileFromPath(buffer);

    loadPropertyList(&buffer[0], &tileProperties);

    for (int c = 0; c < 256; ++c) {
    
        if (getFromMap(&tileProperties, c)) {
            const struct CTile3DProperties prop = *((struct CTile3DProperties*)getFromMap(&tileProperties, c));
            setInMap(&occluders, c, &occluders);
        } else {
            setInMap(&occluders, c, NULL);
        }
    }

    free(data.data);
}


void loadTexturesForLevel(const uint8_t levelNumber) {
    char tilesFilename[64];

    sprintf(tilesFilename, "tiles%d.lst", levelNumber);
    int c;
    struct StaticBuffer data = loadBinaryFileFromPath(tilesFilename);
    size_t size = data.size;
    char *head = (char *) data.data;
    char *end = head + size;
    char *nameStart = head;
    char buffer[256];

    clearTextures();

    while (head != end) {
        char val = *head;
        if (val == '\n' || val == 0) {
            *head = 0;
            makeTextureFrom(nameStart);
            nameStart = head + 1;
        }
        ++head;
    }

    free(data.data);
    backdrop = loadBitmap("backdrop.img");
    
    //item 0 is a dummy
    for (c = 1; c < itemsCount; ++c) {
        sprintf(&buffer[0], "%s.img", getItem(c)->name);
        itemSprites[c] = (makeTextureFrom(&buffer[0]));
    }
}

void updateCursorForRenderer(const int x, const int z) {
	needsToRedrawVisibleMeshes = TRUE;
	visibilityCached = FALSE;
	cursorX = x;
	cursorZ = z;

	if (x == -1) {
		gunTargetPosition.x = XRES / 4;
	}
}

void tickCamera() {
    static FixP_t One = intToFix(1);
    static FixP_t Two = intToFix(2);
    FixP_t halfOne = Div(One, Two);
    
    if (abs(cameraOffset.mY) <= 1000) {
        cameraOffset.mY = 0;
    }

    if (cameraOffset.mY > 0) {
        cameraOffset.mY -= halfOne;
    } else if (cameraOffset.mZ > 0) {
        cameraOffset.mZ -= halfOne;
    } else if (cameraOffset.mZ < 0) {
        cameraOffset.mZ += halfOne;
    } else if (cameraOffset.mX > 0) {
        cameraOffset.mX -= halfOne;
    } else if (cameraOffset.mX < 0) {
        cameraOffset.mX += halfOne;
    } else if (cameraOffset.mY < 0) {
        cameraOffset.mY += halfOne;
    }
    needsToRedrawVisibleMeshes = TRUE;
}

void drawMap(const uint8_t *  elements,
			 const uint8_t *  items,
			 const uint8_t *  actors,
			 uint8_t *  effects,
			 const struct CActor *  current) {

    const struct Vec2i mapCamera = current->position;
    cameraDirection = current->rotation;
    hasSnapshot = TRUE;

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
	FixP_t standardHeight = Div(intToFix(180), intToFix(100));

	if (!hasSnapshot) {
		return;
	}

	if (playerHeight < playerHeightTarget) {
		playerHeight += playerHeightChangeRate;
	}

	if (abs(gunTargetPosition.y - gunPosition.y) > 3) {
		gunSpeedY = (gunTargetPosition.y > gunPosition.y) ? 4 : -4;
		gunPosition.y += gunSpeedY;
		needsToRedrawVisibleMeshes = TRUE;
	}

	if (gunTargetPosition.x != gunPosition.x) {
		gunSpeedX = (gunTargetPosition.x > gunPosition.x) ? 4 : -4;
		gunPosition.x += gunSpeedX;
		needsToRedrawVisibleMeshes = TRUE;
	}

	if (needsToRedrawVisibleMeshes) {
		char buffer[64];
		char directions[4] = {'N', 'E', 'S', 'W'};
		struct Vec3 tmp;
		FixP_t heightDiff;
		uint8_t lastElement = 0xFF;
        uint8_t itemsSnapshotElement = 0xFF;
		uint8_t element = 0;
		int onTarget;
		struct Vec3 position;
		FixP_t tileHeight = 0;
		int16_t x, y, z;
		int distance;
		FixP_t cameraHeight;
		int c;
		uint8_t facesMask;

		highlightDisplayTime -= ms;
		needsToRedrawVisibleMeshes = FALSE;

		for (c = 0; c < (XRES / 32); ++c) {
			drawBitmap(c * 32, 0, backdrop, FALSE);
		}
		fill(0, YRES / 2, XRES, YRES / 2, 0, FALSE);

		element = map[cameraPosition.y][cameraPosition.x];


        assert(getFromMap(&tileProperties, element));
        
        struct CTile3DProperties tileProp = * ((struct CTile3DProperties*)getFromMap(&tileProperties, element));

		tileHeight = tileProp.mFloorHeight;
		
		cameraHeight = -2 * tileHeight;

		mCamera.mY = cameraHeight - standardHeight;

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
						onTarget = (cursorX == x && cursorZ == z);
						element = map[z][x];

						itemsSnapshotElement = mItems[z][x];

						position.mX =
								mCamera.mX + intToFix(-2 * ((MAP_SIZE - 1) - x));
						position.mY = mCamera.mY;
						position.mZ =
								mCamera.mZ + intToFix(2 * (MAP_SIZE) - (2 * z));

						if (x > 0) {
							facesMask |= (map[z][(x - 1)] != element) ?
										 MASK_RIGHT :
										 0;
						}

						/* remember, bounds - 1! */
						if ((x < (MAP_SIZE - 1)) && (map[z][(x + 1)] == element)) {
							facesMask &= ~MASK_LEFT;
						}

						if ((z < (MAP_SIZE - 1)) && (map[(z + 1)][x] == element)) {
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

						element = map[z][x];
						onTarget = (cursorX == x && cursorZ == z);
						itemsSnapshotElement = mItems[z][x];

						position.mX = mCamera.mX + intToFix(-2 * x);
						position.mY = mCamera.mY;
						position.mZ = mCamera.mZ + intToFix(2 + 2 * z);

						/*						remember, bounds - 1!*/

						if ((x > 0) && (map[z][(x - 1)] == element)) {
							facesMask &= ~MASK_LEFT;
						}

						if ((x < (MAP_SIZE - 1)) && (map[z][(x + 1)] == element)) {
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

						element = map[x][z];
						onTarget = (cursorX == z && cursorZ == x);
						itemsSnapshotElement = mItems[x][z];

						position.mX = mCamera.mX + intToFix(-2 * x);
						position.mY = mCamera.mY;
						position.mZ = mCamera.mZ + intToFix(2 - 2 * z);

						/* remember, bounds - 1! */

						if ((x > 0) && (map[(x - 1)][z] == element)) {
							facesMask &= ~MASK_LEFT;
						}

						if ((x < (MAP_SIZE - 1)) && (map[(x + 1)][z] == element)) {
							facesMask &= ~MASK_RIGHT;
						}

						if ((z < (MAP_SIZE - 1)) && (map[x][(z + 1)] == element)) {
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

                        element = map[x][z];
						onTarget = (cursorX == z && cursorZ == x);
						itemsSnapshotElement = mItems[x][z];

						position.mX = mCamera.mX + intToFix(2 * x);
						position.mY = mCamera.mY;
						position.mZ = mCamera.mZ + intToFix(2 * (z - MAP_SIZE + 1));


						/* remember, bounds - 1! */
						if ((x > 0) && (map[(x - 1)][z] == element)) {
							facesMask &= ~MASK_RIGHT;
						}

						if ((x < (MAP_SIZE - 1)) && (map[(x + 1)][z] == element)) {
							facesMask &= ~MASK_LEFT;
						}

						if ((z < (MAP_SIZE - 1)) && (map[x][(z - 1)] == element)) {
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

                assert(getFromMap(&tileProperties, element) != NULL);

                if (lastElement != element) {
                    tileProp = *((struct CTile3DProperties*)getFromMap(&tileProperties, element));
                }

                FixP_t heightDiff = tileProp.mCeilingHeight - tileProp.mFloorHeight;
                lastElement = element;


				if (tileProp.mFloorRepeatedTextureIndex != 0xFF
					&& tileProp.mFloorRepetitions > 0) {

					switch (tileProp.mGeometryType) {
						case kRightNearWall:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mFloorHeight * 2)
									   - intToFix(tileProp.mFloorRepetitions)),
									  zero);

							drawRightNear(
									tmp, intToFix(tileProp.mFloorRepetitions),
									nativeTextures[tileProp.mFloorRepeatedTextureIndex],
									facesMask, TRUE);

							break;

						case kLeftNearWall:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mFloorHeight * 2)
									   - intToFix(tileProp.mFloorRepetitions)),
									  zero);

							drawLeftNear(
									tmp, intToFix(tileProp.mFloorRepetitions),
									nativeTextures[tileProp.mFloorRepeatedTextureIndex], facesMask, TRUE);
							break;

						case kCube:
						default:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mFloorHeight * 2)
									   - intToFix(tileProp.mFloorRepetitions)),
									  zero);

							drawColumnAt(
									tmp, intToFix(tileProp.mFloorRepetitions),
									nativeTextures[tileProp.mFloorRepeatedTextureIndex],
									facesMask, FALSE, TRUE);
							break;
					}
				}

				if (tileProp.mCeilingRepeatedTextureIndex != 0xFF
					&& tileProp.mCeilingRepetitions > 0) {

					switch (tileProp.mGeometryType) {
						case kRightNearWall:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mCeilingHeight * 2)
									   + intToFix(tileProp.mCeilingRepetitions)),
									  zero);

							drawRightNear(
									tmp, intToFix(tileProp.mCeilingRepetitions),
									nativeTextures[tileProp.mCeilingRepeatedTextureIndex],
									facesMask, TRUE);
							break;

						case kLeftNearWall:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mCeilingHeight * 2)
									   + intToFix(tileProp.mCeilingRepetitions)),
									  zero);

							drawLeftNear(
									tmp, intToFix(tileProp.mCeilingRepetitions),
									nativeTextures[tileProp.mCeilingRepeatedTextureIndex],
									facesMask, TRUE);
							break;

						case kCube:
						default:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mCeilingHeight * 2)
									   + intToFix(tileProp.mCeilingRepetitions)),
									  zero);

							drawColumnAt(
									tmp, intToFix(tileProp.mCeilingRepetitions),
									nativeTextures[tileProp.mCeilingRepeatedTextureIndex],
									facesMask, FALSE, TRUE);
							break;
					}
				}

				if (tileProp.mFloorTextureIndex != 0xFF) {

					tmp.mX = position.mX;
					tmp.mY = position.mY;
					tmp.mZ = position.mZ;

					addToVec3(&tmp, 0, (tileProp.mFloorHeight * 2), 0);


					drawFloorAt(tmp, nativeTextures[tileProp.mFloorTextureIndex], cameraDirection);
				}

				if (tileProp.mCeilingTextureIndex != 0xFF) {

                    enum EDirection newDirection = cameraDirection;

					tmp.mX = position.mX;
					tmp.mY = position.mY;
					tmp.mZ = position.mZ;

					addToVec3(&tmp, 0, (tileProp.mCeilingHeight * 2), 0);

					if (cameraDirection == kNorth) {
						newDirection = kSouth;
					}
					if (cameraDirection == kSouth) {
						newDirection = kNorth;
					}

					drawCeilingAt(
							tmp, nativeTextures[tileProp.mCeilingTextureIndex], newDirection);
				}

				if (tileProp.mGeometryType != kNoGeometry
					&& tileProp.mMainWallTextureIndex != 0xFF) {

					int integerPart = fixToInt(tileProp.mCeilingHeight)
									  - fixToInt(tileProp.mFloorHeight);

					FixP_t adjust = 0;

					if (((heightDiff * 2) - intToFix(integerPart)) >= halfOne) {
						adjust = Div(halfOne, four);
					}

					switch (tileProp.mGeometryType) {
						case kRightNearWall:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mFloorHeight * 2) + heightDiff),
									  zero);

							drawRightNear(
									tmp, (heightDiff + Div(adjust, two)),
									nativeTextures[tileProp.mMainWallTextureIndex],
									facesMask, tileProp.mRepeatMainTexture);
							break;

						case kLeftNearWall:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mFloorHeight * 2) + heightDiff),
									  zero);

							drawLeftNear(
									tmp, (heightDiff + Div(adjust, two)),
									nativeTextures[tileProp.mMainWallTextureIndex],
									facesMask, tileProp.mRepeatMainTexture);
							break;

						case kCube:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp.mFloorHeight * 2) + heightDiff),
									  zero);

							drawColumnAt(tmp, (heightDiff + Div(adjust, two)),
										 nativeTextures[tileProp.mMainWallTextureIndex],
										 facesMask, tileProp.mNeedsAlphaTest,
										 tileProp.mRepeatMainTexture);
						default:
							break;
					}
				}

                if (itemsSnapshotElement != 0xFF && itemsSnapshotElement != 0) {
                    tmp.mX = position.mX;
                    tmp.mY = position.mY;
                    tmp.mZ = position.mZ;
                    
                    addToVec3(&tmp, 0, (tileProp.mFloorHeight * 2) + one, 0);
                    
                    drawBillboardAt(tmp, itemSprites[itemsSnapshotElement]->rotations[0], one, 32);
                }

			}
		}

		enter2D();

		if (shouldShowDamageHighlight) {

			fill(0, 0, XRES, YRES, getPaletteEntry(0xFF0000FF), TRUE);

			if (highlightDisplayTime <= 0) {
				shouldShowDamageHighlight = FALSE;
			}

			needsToRedrawVisibleMeshes = TRUE;
		}

		if (shouldShowDetectedHighlight) {

			fill(0, 0, XRES, YRES, getPaletteEntry(0xFF00FFFF), TRUE);
			if (highlightDisplayTime <= 0) {
				shouldShowDetectedHighlight = FALSE;
			}
			needsToRedrawVisibleMeshes = TRUE;
		}

		fill(0, 0, XRES_FRAMEBUFFER, 8, 0, FALSE);
		drawTextAt(2, 1, "Agent in the field", 255);


		drawTextAt(34, 1, "Map", 255);

		fill(XRES, 8, XRES_FRAMEBUFFER - XRES, YRES_FRAMEBUFFER - 8, 255, FALSE);


		for (y = 0; y < MAP_SIZE; ++y) {
			for (x = 0; x < MAP_SIZE; ++x) {
				uint8_t tile = map[y][x];

				if (!revealed[y][x]) {
					linesOfSight[y][x] = FALSE;
					continue;
				}

				fill(256 + (x), 16 + (2 * y), 1, 2, 64, FALSE);

				if (getFromMap(&colliders, tile)) {
					fill(256 + (x), 16 + (2 * y), 1, 2, 0, FALSE);
				}

				if (linesOfSight[y][x]) {
					fill(256 + (x), 16 + (2 * y), 1, 2, 128, FALSE);
				}

				if (cursorZ == y && x == cursorX) {
					fill(256 + (x), 16 + (2 * y), 1, 2, 255, FALSE);
				}

				if (tile == 'K' || tile == 'E' || tile == 'i') {
					fill(256 + (x), 16 + (2 * y), 1, 2, 128, FALSE);
				}

				linesOfSight[y][x] = FALSE;
			}
		}

		drawRect(256, 16, 63, 128, 0);

		fill(256, 16 + (2 * cameraPosition.y), (cameraPosition.x), 1,
			 (cameraDirection == kWest) ? 48 : 32, FALSE);

		fill(256 + (cameraPosition.x), 16 + (2 * cameraPosition.y),
			 XRES_FRAMEBUFFER - (256 + (cameraPosition.x)), 1,
			 (cameraDirection == kEast) ? 48 : 32, FALSE);

		fill(256 + (cameraPosition.x), 16, 1, (2 * cameraPosition.y),
			 (cameraDirection == kNorth) ? 48 : 32, FALSE);

		fill(256 + (cameraPosition.x), 16 + (2 * cameraPosition.y), 1,
			 96 - (16 + (2 * cameraPosition.y)),
			 (cameraDirection == kSouth) ? 48 : 32, FALSE);

		fill(256 + (cameraPosition.x), 16 + (2 * cameraPosition.y), 1,
			 2, 48, FALSE);


		sprintf(buffer, "HP: %d \nAmmo: %d", playerHealth, playerAmmo);
		drawTextAt(34, 22, buffer, 15);
		sprintf(buffer, "Dir: %c", directions[(int) (cameraDirection)]);


		drawTextAt(34, 24, buffer, 0);

		if (covered) {
			drawTextAt(34, 21, "Covered", 15);
		}
	}
}
