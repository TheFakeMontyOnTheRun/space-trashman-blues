#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef N64
#include <libdragon.h>
#endif

const long UCLOCKS_PER_SEC = 1000;

long timeEllapsed = 0;

long uclock() {
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
#include "CRenderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "Derelict.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

void initHW(int argc, char **argv) {
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

#ifdef __EMSCRIPTEN__
void mainLoop();
#endif

#ifndef ANDROID


int main(int argc, char **argv) {

    initHW(argc, argv);

    enterState(kMainMenu);

    end_clock = uclock();
    prev = 0;
    start_clock = uclock();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else

    while (isRunning) {

        long now, delta_time;

        now = (end_clock - start_clock) / (UCLOCKS_PER_SEC / 1000);
        delta_time = now - prev;
        prev = now;

        if (delta_time < 50) {
            delta_time = 50;
        }

#ifndef N64
#ifndef NDS
        startFrameGL(640, 480);
#else
        startFrameGL(255, 191);
#endif
#else
        startFrameGL(320, 240);
#endif

        isRunning = isRunning && menuTick(20);

        endFrameGL();
        flipRenderer();

    }
#endif


    shutdownHW();

    return 0;
}

#endif
