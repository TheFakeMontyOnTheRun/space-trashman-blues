#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef N64
#include <libdragon.h>
#endif

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "Derelict.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

char *textBuffer;
extern char *messageLogBuffer;
extern enum EVisibility *visMap;
extern struct Vec2i *distances;
extern uint8_t *collisionMap;

void initHW(int argc, char **argv) {
    textBuffer = (char *) allocMem(TEXT_BUFFER_SIZE, GENERAL_MEMORY, 1);
    messageLogBuffer = (char *) allocMem(256, GENERAL_MEMORY, 1);
    collisionMap = (uint8_t *) allocMem(256, GENERAL_MEMORY, 1);
    visMap = (enum EVisibility *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(enum EVisibility), GENERAL_MEMORY, 1);
    distances = (struct Vec2i *) allocMem(2 * MAP_SIZE * MAP_SIZE * sizeof(struct Vec2i), GENERAL_MEMORY, 1);
    itemsInMap = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);
    map = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);

#ifndef N64
#ifndef NDS
    initFileReader("base.pfs");
#else
    nitroFSInit(argv[0]);
    initFileReader("nitro:/base.pfs");
#endif
#else
    dfs_init(DFS_DEFAULT_LOCATION);
    initFileReader("rom:/base.pfs");
#endif
    graphicsInit();
}

void shutdownHW(void) {
    graphicsShutdown();
}

long start_clock, end_clock, prev;

int main(int argc, char **argv) {

    initHW(argc, argv);

    enterState(kMainMenu);

    end_clock = 0;
    prev = 0;
    start_clock = 0;

    while (isRunning) {

        long now, delta_time;

        now = (end_clock - start_clock);
        delta_time = now - prev;
        prev = now;

        if (delta_time < 20) {
            delta_time = 20;
        }

        startFrame(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER);
        isRunning = isRunning && menuTick(delta_time);
        endFrame();
        flipRenderer();

    }


    shutdownHW();

    return 0;
}
