#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
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

    if (shouldContinue != kCrawlerGameInProgress) {
        gameTicks = 0;
    }
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

    sprintf (nameBuffer, "map%d.img", map);

    if (mapTopLevel[0]) {
		for (c = 0; c < 8; ++c ) {
#ifndef N64
			free(mapTopLevel[c]);
#else
			free_uncached(mapTopLevel[c]);
#endif
		}
    }

	for (c  = 0; c < 8; ++c ) {
		char buffer[32];
		sprintf(buffer, "map%d_tile%04d.img", map, c);
		mapTopLevel[c] = loadBitmap(buffer);
	}

#ifndef N64
    free(buffer.data);
#else
	free_uncached(buffer.data);
#endif
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
                                                                     elements[(actor.position.y * MAP_SIZE) +
                                                                              actor.position.x]))->mFloorHeight;
        } else {
            yCameraOffset = 0;
        }

        actor.position.x = x;
        actor.position.y = z;
        actor.rotation = (enum EDirection) (rotation);

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
