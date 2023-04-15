#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


#include <kernel.h>
#include <stdlib.h>
#include <malloc.h>
#include <tamtypes.h>
#include <math3d.h>
#include <packet.h>
#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>
#include <dma.h>
#include <graph.h>
#include <draw.h>
#include <draw3d.h>
#include <gs_gp.h>

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


extern packet_t *packets[2];
extern packet_t *current;
extern MATRIX local_world;
extern MATRIX world_view;
extern MATRIX view_screen;
extern int context;
extern zbuffer_t zBuffer;
extern framebuffer_t frame;
extern qword_t *dmatag;

extern MATRIX local_screen;
extern packet_t *current;
extern qword_t *_q;
extern xyz_t *verts;
extern color_t *colors;
extern VECTOR *temp_vertices;
extern int vertex_count;
extern prim_t prim;
extern color_t color;



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
uint8_t mItems[MAP_SIZE][MAP_SIZE];
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

enum EVisibility visMap[MAP_SIZE * MAP_SIZE];
struct Vec2i distances[2 * MAP_SIZE * MAP_SIZE];

uint32_t getPaletteEntry(const uint32_t origin) {
    return (0x80 << 24) + (origin & 0x00FFFFFF);
}

void enter2D(void) {
}

void initGL() {
}

void clearRenderer() {
}

void startFrameGL(int width, int height) {

	current = packets[context];

	// Grab our dmatag pointer for the dma chain.
	dmatag = current->data;

	// Now grab our qword pointer and increment past the dmatag.
	_q = dmatag;
	_q++;

    PACK_GIFTAG(_q,GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
    _q++;
    PACK_GIFTAG(_q, GS_SET_TEST(DRAW_ENABLE,ATEST_METHOD_NOTEQUAL,0x00,ATEST_KEEP_FRAMEBUFFER,
                               DRAW_DISABLE,DRAW_DISABLE,
                               DRAW_ENABLE,ZTEST_METHOD_ALLPASS), GS_REG_TEST + 0);
    _q++;


    _q = draw_clear(_q, 0, 2048.0f - 320.0f, 2048.0f - 256.0f, frame.width, frame.height, 0, 0, 0);

    PACK_GIFTAG(_q,GIF_SET_TAG(1,0,0,0,GIF_FLG_PACKED,1), GIF_REG_AD);
    _q++;
    PACK_GIFTAG(_q, GS_SET_TEST(DRAW_ENABLE,ATEST_METHOD_EQUAL,0x80,ATEST_KEEP_ALL,
                               DRAW_DISABLE,DRAW_DISABLE,
                               DRAW_ENABLE,ZTEST_METHOD_GREATER_EQUAL), GS_REG_TEST + 0);
    _q++;


    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = FALSE;
    enter2D();

	VECTOR camera_position = {0.00f, -3.00f, 2, 1.00f};
	VECTOR camera_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

	// Create the world_view matrix.
	create_world_view(world_view, camera_position, camera_rotation);
}

void endFrameGL() {
}

void enter3D(void) {
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

	free(data.data);
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
    
	head = (char *) data.data;
	end = head + data.size;
	nameStart = head;

	clearTextures();

    //item 0 is a dummy
    for (int c = 1; c < itemsCount; ++c) {
        sprintf(&buffer[0], "%s.img", getItem(c)->name);
        itemSprites[c] = (makeTextureFrom(&buffer[0]));
    }

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

	free(data.data);
}

void updateCursorForRenderer(const int x, const int z) {
	needsToRedrawVisibleMeshes = TRUE;
	visibilityCached = FALSE;
	cursorX = x;
	cursorZ = z;
}

void drawMap(const uint8_t *  elements,
             const uint8_t *  items,
             const uint8_t *  actors,
             uint8_t *  effects,
             const struct CActor *  current) {

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

	for (z = 0; z < MAP_SIZE; ++z) {
		for (x = 0; x < MAP_SIZE; ++x) {
			const uint16_t offset = (MAP_SIZE * z) + x;
			const uint8_t actor = actors[offset];
			const uint8_t item = items[offset];
			const uint8_t effect = effects[offset];

			mItems[z][x] = 0xFF;

			if (item != 0xFF) {
				mItems[z][x] = item;
			}
		}
	}

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

	if (needsToRedrawVisibleMeshes) {
		char buffer[64];
		char directions[4] = {'N', 'E', 'S', 'W'};
		struct Vec3 tmp;
		struct CTile3DProperties *tileProp;
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

		needsToRedrawVisibleMeshes = FALSE;

		element = map[cameraPosition.y][cameraPosition.x];

		tileProp = ((struct CTile3DProperties *) getFromMap(&tileProperties,
															element));

		if (tileProp) {
			tileHeight = tileProp->mFloorHeight;
		}

		cameraHeight = -2 * tileHeight;

		mCamera.mY = cameraHeight - standardHeight;

		enter3D();

		for (distance = 0; distance < (MAP_SIZE + MAP_SIZE); ++distance ) {
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
									  ((tileProp->mFloorHeight)
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
									  ((tileProp->mFloorHeight)
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
									  ((tileProp->mFloorHeight)
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
									  ((tileProp->mCeilingHeight)
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
									  ((tileProp->mCeilingHeight)
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
									  ((tileProp->mCeilingHeight)
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

					addToVec3(&tmp, 0, (tileProp->mFloorHeight), 0);


					drawFloorAt(tmp, nativeTextures[tileProp->mFloorTextureIndex], cameraDirection);
				}

				if (tileProp->mCeilingTextureIndex != 0xFF) {

					uint8_t newDirection = cameraDirection;

					tmp.mX = position.mX;
					tmp.mY = position.mY;
					tmp.mZ = position.mZ;

					addToVec3(&tmp, 0, (tileProp->mCeilingHeight), 0);

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
					struct Vec3 tmp2;
					int integerPart = fixToInt(tileProp->mCeilingHeight)
									  - fixToInt(tileProp->mFloorHeight);

					FixP_t adjust = 0;

					if (((heightDiff) - intToFix(integerPart)) >= halfOne) {
						adjust = Div(halfOne, four);
					}

					switch (tileProp->mGeometryType) {
						case kWallNorth:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp->mFloorHeight) + heightDiff),
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
									  ((tileProp->mFloorHeight) + heightDiff),
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
									  ((tileProp->mFloorHeight) + heightDiff),
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
									  ((tileProp->mFloorHeight) + heightDiff),
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
									  ((tileProp->mFloorHeight) + heightDiff),
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

							addToVec3(&tmp, 0, (tileProp->mFloorHeight), 0);
							addToVec3(&tmp2, 0, (tileProp->mCeilingHeight), 0);

							drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection, flipTextureVertical);
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

							addToVec3(&tmp2, 0, (tileProp->mFloorHeight), 0);
							addToVec3(&tmp, 0, (tileProp->mCeilingHeight), 0);

							drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex], cameraDirection, flipTextureVertical);
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

							addToVec3(&tmp2, 0, (tileProp->mFloorHeight), 0);
							addToVec3(&tmp, 0, (tileProp->mCeilingHeight), 0);

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


							addToVec3(&tmp2, 0, (tileProp->mFloorHeight), 0);
							addToVec3(&tmp, 0, (tileProp->mCeilingHeight), 0);

							drawRampAt(tmp, tmp2, nativeTextures[tileProp->mMainWallTextureIndex],
									   (cameraDirection + 3) & 3, flipTextureVertical);
						}
							break;
						case kCube:

							tmp.mX = position.mX;
							tmp.mY = position.mY;
							tmp.mZ = position.mZ;

							addToVec3(&tmp, zero,
									  ((tileProp->mFloorHeight) + heightDiff),
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

                    addToVec3(&tmp, 0, (tileProp->mFloorHeight) + one, 0);

                    drawBillboardAt(tmp, itemSprites[itemsSnapshotElement], one, 32);
                }

			}
		}

		enter2D();

		if (focusItemName != NULL) {
			size_t len = strlen(focusItemName);
			int lines = 1 + (len / 27);
			fill( 0, YRES - (8 * lines), XRES, lines * 8, 0, 1 );
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
