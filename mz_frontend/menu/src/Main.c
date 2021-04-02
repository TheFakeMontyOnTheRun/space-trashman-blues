#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"

void initHW() {
#ifdef CD32
    initFileReader("base5.pfs");
#else
#ifndef AGA5BPP
    initFileReader("base.pfs");
#else
    initFileReader("base4.pfs");
#endif
#endif

    graphicsInit();
    stateTick = 0;
    globalTick = 0;
}

void shutdownHW() {
    graphicsShutdown();
}

extern unsigned long t0, t1, t2;

int main(int argc, char **argv) {

    initHW();
    enterState(kMainMenu);

    while (isRunning) {
        mainLoop();
    }
    unloadStateCallback();
    shutdownHW();

    printf("t0 = %zu, t1 = %zu, t2 = %zu\n", t0, t1, t2);

    return 0;
}