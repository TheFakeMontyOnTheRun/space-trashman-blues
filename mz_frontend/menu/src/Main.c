#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef AMIGA
#include "AmigaInt.h"

#define VERSTAG "\0$VER: Sub Mare Imperium - Derelict 1.0 (10.10.2023)"
#else

#ifdef WIN32
#include <windows.h>
#include "Win32Int.h"

#ifdef MSVC
#include "resource.h"
#endif

#else

#include <stdint.h>

#endif


#endif

#ifdef WIN32
const long UCLOCKS_PER_SEC = 1000;
long uclock() {
    SYSTEMTIME systime;
    GetSystemTime(&systime);


    return 1000 * ((systime.wSecond * 1000) + (systime.wMilliseconds));
}
#else
#ifndef __DJGPP__
const long UCLOCKS_PER_SEC = 1000;

long timeEllapsed = 0;

long uclock() {
    timeEllapsed += (1000 / 60);
    return timeEllapsed;
}

#endif
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


char *textBuffer;
/*
extern char *messageLogBuffer;
extern enum EVisibility *visMap;
extern struct Vec2i *distances;
extern uint8_t *collisionMap;
extern struct Texture *textures;

struct Texture internalTexturesMem[TOTAL_TEXTURES];
*/
void initHW(int argc, char** argv) {

    textBuffer = (char *) allocMem(TEXT_BUFFER_SIZE, GENERAL_MEMORY, 1);
/*
    messageLogBuffer = (char *) allocMem(256, GENERAL_MEMORY, 1);
    collisionMap = (uint8_t *) allocMem(256, GENERAL_MEMORY, 1);
    visMap = (enum EVisibility *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(enum EVisibility), GENERAL_MEMORY, 1);
    distances = (struct Vec2i *) allocMem(2 * MAP_SIZE * MAP_SIZE * sizeof(struct Vec2i), GENERAL_MEMORY, 1);
    textures = &internalTexturesMem[0];
    itemsInMap = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);
*/

    initFileReader("base.pfs");

    graphicsInit();
}

void shutdownHW() {
    graphicsShutdown();
/*
    disposeMem(textBuffer);
    disposeMem(messageLogBuffer);
    disposeMem(collisionMap);
    disposeMem(visMap);
    disposeMem(distances);
    disposeMem(itemsInMap);
    disposeMem(map);
    */
}

long start_clock, end_clock, prev;


#ifdef ATARIST
int doMain(void);

int main(int argc, char **argv) {
    Supexec(&doMain);
    return 0;
}

int doMain(void) {
#else
int main(int argc, char **argv) {
#endif
    initHW(argc, argv);
    initStation();
    enterState(kMainMenu);

    end_clock = uclock();
    prev = 0;

    start_clock = uclock();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (isRunning) {
        long now, delta_time;

#ifdef AMIGA
#ifdef AGA8BPP
        delta_time = 50;
#else
        delta_time = 1000;
#endif

#else
        now = (end_clock - start_clock) / (UCLOCKS_PER_SEC / 1000);
        delta_time = now - prev;
        prev = now;

        /* protect against machines too fast for their own good. */
        if (delta_time < 50) {
            delta_time = 50;
        }
#endif
        isRunning = isRunning && menuTick(delta_time);
        flipRenderer();
    }
#endif
    unloadStateCallback(-1);
    shutdownHW();

    return 0;
}
