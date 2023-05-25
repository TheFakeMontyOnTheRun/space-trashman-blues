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
#include "Globals.h"

int x = 0;
int z = 0;
int rotation = 0;
enum CrawlerState shouldContinue = kCrawlerGameInProgress;
struct CActor actor;

void clearMapCache() {
    memset (&(ITEMS_IN_MAP(0, 0)), 0xFF, MAP_SIZE * MAP_SIZE);
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

void setItem(const int x, const int y, uint8_t item) {

    ITEMS_IN_MAP(x, y) = item;
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

    mapTopLevel = loadBitmap(nameBuffer);

    free(buffer.data);
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
                                                                     LEVEL_MAP(x, z) ))->mFloorHeight -
                            ((struct CTile3DProperties *) getFromMap(&tileProperties,
                                                                     LEVEL_MAP(actor.position.x, actor.position.y)))->mFloorHeight;
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
        drawMap(&actor);
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
    needsToRedrawHUD = TRUE;
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
