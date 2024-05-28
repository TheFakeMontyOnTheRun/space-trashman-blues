#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

const long UCLOCKS_PER_SEC = 1000;

long timeEllapsed = 0;

long uclock(void) {
    timeEllapsed += (1000 / 60);
    return timeEllapsed;
}

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "CTile3DProperties.h"
#include "Renderer.h"
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
extern struct Texture *nativeTextures[TOTAL_TEXTURES];

void initHW(int argc, char** argv) {
    textBuffer = (char *) allocMem(TEXT_BUFFER_SIZE, GENERAL_MEMORY, 1);
    messageLogBuffer = (char *) allocMem(256, GENERAL_MEMORY, 1);
    collisionMap = (uint8_t *) allocMem(256, GENERAL_MEMORY, 1);
    visMap = (enum EVisibility *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(enum EVisibility), GENERAL_MEMORY, 1);
    distances = (struct Vec2i *) allocMem(2 * MAP_SIZE * MAP_SIZE * sizeof(struct Vec2i), GENERAL_MEMORY, 1);
    itemsInMap = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);
    map = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);
    memFill(&nativeTextures[0], 0, sizeof(struct Texture) * TOTAL_TEXTURES);

    initFileReader("base.pfs");
    graphicsInit();
}

void shutdownHW(void) {
    graphicsShutdown();
}

long start_clock, end_clock, prev;

#ifdef __EMSCRIPTEN__
void mainLoop (void) {
    if (enable3DRendering) {
        startFrame(0, 0, 640, 480);
        menuTick(20);
        endFrame();
        flipRenderer();
    }
}
#endif



int main(int argc, char **argv) {

    initStation();

    initHW(0, NULL);

    enterState(kMainMenu);

    end_clock = uclock();
    prev = 0;
    start_clock = uclock();

    clearRenderer();
    enable3DRendering = TRUE;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 50, 1);
#else
    while (isRunning) {

        long now, delta_time;

        now = (end_clock - start_clock) / (UCLOCKS_PER_SEC / 1000);
        delta_time = now - prev;
        prev = now;

        if (delta_time < 50) {
            delta_time = 50;
        }

        startFrame(0, 0, 640, 480);

        isRunning = isRunning && menuTick(10);

        endFrame();
        flipRenderer();

    }
#endif
    unloadStateCallback(-1);

    shutdownHW();

    return 0;
}
