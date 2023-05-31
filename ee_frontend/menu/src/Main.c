#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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


void initHW() {
    initFileReader("base.pfs");
    graphicsInit();
}

void shutdownHW() {
    graphicsShutdown();
}

long start_clock, end_clock, prev;


int main(int argc, char **argv) {

	initStation();

    initHW();

    enterState(kMainMenu);

    end_clock = uclock();
    prev = 0;
    start_clock = uclock();

	clearRenderer();

    while (isRunning) {

        long now, delta_time;

        now = (end_clock - start_clock) / (UCLOCKS_PER_SEC / 1000);
        delta_time = now - prev;
        prev = now;

        if (delta_time < 50) {
            delta_time = 50;
        }

		startFrameGL(640, 480);

        isRunning = isRunning && menuTick(10);

		endFrameGL();
		flipRenderer();

	}

    unloadStateCallback();

    shutdownHW();

    return 0;
}
