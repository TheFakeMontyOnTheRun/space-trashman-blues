#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdint.h>
#include <unistd.h>

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "CPackedFileReader.h"
#include "Dungeon.h"
#include "SoundSystem.h"

uint8_t elements[(MAP_SIZE * MAP_SIZE)];
uint8_t actorsInMap[(MAP_SIZE * MAP_SIZE)];
uint8_t items[(MAP_SIZE * MAP_SIZE)];
uint8_t effects[(MAP_SIZE * MAP_SIZE)];
int x = 0;
int z = 0;
int rotation = 0;
enum CrawlerState shouldContinue = kCrawlerGameInProgress;
struct CActor actor;

int loopTick(enum ECommand command);

void clearMapCache() {
    size_t sizeForSet = sizeof(uint8_t) * (MAP_SIZE * MAP_SIZE);
    memset (&items[0], 0xFF, sizeForSet);
    memset (&actorsInMap[0], 0xFF, sizeForSet);
    memset (&effects[0], 0xFF, sizeForSet);
}

void onLevelLoaded(int index) {
    clearMapCache();
    shouldContinue = kCrawlerGameInProgress;
    clearMap(&tileProperties);
    loadTexturesForLevel(index);
    loadTileProperties(index);
}

void tickMission(enum ECommand cmd) {

    struct GameSnapshot snapshot = dungeon_tick(cmd);

    x = snapshot.camera_x;
    z = snapshot.camera_z;
    rotation = snapshot.camera_rotation;
    shouldContinue = snapshot.should_continue;

    updateCursorForRenderer(snapshot.playerTarget.x, snapshot.playerTarget.y);

    if (playerHealth <= 0) {
        shouldContinue = kCrawlerGameOver;
    }

    if (shouldContinue != kCrawlerGameInProgress) {
        gameTicks = 0;
    }
}

void addEffectSprite(const int x, const int y, const int billboard) {
    effects[(y * MAP_SIZE) + x] = billboard;
    playSound(ENEMY_FIRING_GUN);
}

void setElement(const int x, const int y, uint8_t element) {
    elements[(MAP_SIZE * y) + x] = element;
}

void setActor(const int x, const int y, uint8_t actor) {
    actorsInMap[(MAP_SIZE * y) + x] = actor;
}

void setItem(const int x, const int y, uint8_t item) {
    items[(MAP_SIZE * y) + x] = item;
}

void setDamage() {
}

void setDetected() {
    /* enemy detected you */
    playSound(PLAYER_GOT_DETECTED_SOUND);
}

uint8_t getItemOnMap(int x, int y) {
    return items[(MAP_SIZE * y) + x];
}

void loadMap(int map, struct MapWithCharKey *collisionMap) {

    /* all the char keys plus null terminator */
    char collisions[256 + 1];
    int c;
    char nameBuffer[256];
    struct StaticBuffer buffer;

    collisions[256] = 0;
    for (c = 0; c < 256; ++c) {
        collisions[c] = (getFromMap(collisionMap, c) != NULL) ? '1' : '0';
    }

    /* 16 bytes should be enough here... */

    sprintf (nameBuffer, "map%d.txt", map);
    buffer = loadBinaryFileFromPath(nameBuffer);
    dungeon_loadMap(buffer.data, collisions, map);
    free(buffer.data);
}

int canSeeSpy(const struct Vec2i seer,
              int direction,
              const struct Vec2i target,
              int enemy) {
    int iX;
    int iY;
    char tile;
    int occluder;
    int dx = (target.x - seer.x);
    int dy = (target.y - seer.y);
    FixP_t x = intToFix(seer.x);
    FixP_t y = intToFix(seer.y);
    FixP_t one = intToFix(1);
    FixP_t zero = 0;
    FixP_t incX;
    FixP_t incY;
    FixP_t targetX = intToFix(target.x);
    FixP_t targetY = intToFix(target.y);

    /* we must pick the bigger, iterate on each square of it and slowly increment the smaller */

    if (abs(dx) >= abs(dy)) {

        /* ++x, y += incY */
        FixP_t inc = zero;

        if (dx != 0) {
            inc = Div(intToFix(abs(dy)), intToFix(abs(dx)));

            if ((direction == 0 || direction == 2)) {
                return FALSE;
            }

            if (dy < 0) {
                inc = -inc;
            }
        }

        incY = inc;

        if (dx >= 0) {
            incX = one;
        } else {
            incX = -one;
        }

        if ((direction == 3) && incX > zero) {
            return FALSE;
        }
        if ((direction == 1) && incX < zero) {
            return FALSE;
        }

    } else {
        /* ++y, x += incX */
        FixP_t inc = zero;

        if (dy != 0) {
            inc = Div(intToFix(abs(dx)), intToFix(abs(dy)));

            if ((direction == 1 || direction == 3)) {
                return FALSE;
            }

            if (dx < 0) {
                inc = -inc;
            }
        }

        incX = inc;

        if (dy >= 0) {
            incY = one;
        } else {
            incY = -one;
        }

        if ((direction == 2) && incY < zero) {
            return FALSE;
        }

        if ((direction == 0) && incY > zero) {
            return FALSE;
        }
    }

    do {
        x += incX;
        y += incY;

        iX = fixToInt(x);
        iY = fixToInt(y);

        tile = visibleElementsMap[(iY * MAP_SIZE) + iX];

        if (tile == 0) {
            /* map is not loaded  yet... */
            return FALSE;
        }

        occluder = (getFromMap(&enemySightBlockers, tile) != NULL);

        if (occluder) {
            return FALSE;
        }

        if ((abs(x - targetX) < one) && (abs(y - targetY) < one)) {
            return TRUE;
        }

        linesOfSight[iY][iX] = TRUE;

    } while (!occluder
             && (iX < MAP_SIZE && iY < MAP_SIZE && iX >= 0 && iY >= 0));

    return FALSE;
}

void renderTick(long ms) {
    render(ms);
}

int loopTick(enum ECommand command) {

    int needRedraw = 0;

    if (command == kCommandBack) {
        shouldContinue = kCrawlerQuit;
    } else if (command != kCommandNone || gameTicks == 0) {

        if (command == kCommandFire1 || command == kCommandFire2
            || command == kCommandFire3 || command == kCommandFire4) {

            visibilityCached = FALSE;
        }

        tickMission(command);

        if (gameTicks != 0) {
            yCameraOffset = ((struct CTile3DProperties *) getFromMap(&tileProperties,
                                                                     elements[(z * MAP_SIZE) + x]))->mFloorHeight -
                            ((struct CTile3DProperties *) getFromMap(&tileProperties,
                                                                     elements[(actor.mPosition.y * MAP_SIZE) +
                                                                              actor.mPosition.x]))->mFloorHeight;
        } else {
            yCameraOffset = 0;
        }

        actor.mPosition.x = x;
        actor.mPosition.y = z;
        actor.mDirection = (enum EDirection) (rotation);

        needRedraw = 1;
    }

    if (zCameraOffset != 0 || xCameraOffset != 0 || yCameraOffset != 0) {
        needRedraw = 1;
    }


    if (needRedraw) {
        drawMap(&elements[0], &items[0], &actorsInMap[0], &effects[0],
                &actor);
        if (!enable3DRendering) {
            enable3DRendering = TRUE;
            visibilityCached = FALSE;
        }
    }
    return shouldContinue;
}

void initRoom(int room) {
    int16_t c;

    shouldContinue = kCrawlerGameInProgress;
    mBufferedCommand = kCommandNone;
    gameTicks = 0;
    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = TRUE;
    onLevelLoaded(room);

    for (c = 0; c < 256; ++c) {

        struct CTile3DProperties *tile3DProperties =
                (struct CTile3DProperties *) getFromMap(&tileProperties, c);

        if (tile3DProperties) {
            setInMap(&colliders, c,
                     tile3DProperties->mBlockMovement ? &colliders : NULL);
        } else {
            setInMap(&colliders, c, NULL);
        }
    }

    loadMap(room, &colliders);
}
