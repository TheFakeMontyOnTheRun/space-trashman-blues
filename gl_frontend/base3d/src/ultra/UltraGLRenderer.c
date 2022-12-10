#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <libdragon.h>
#include <GL/gl.h>
#include <GL/gl_integration.h>
#include <malloc.h>
#include <math.h>

#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Globals.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Core.h"
#include "Engine.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"

int snapshotSignal = '.';

void graphicsInit() {

	debug_init_isviewer();
	debug_init_usblog();

	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE_FETCH_ALWAYS);

	defaultFont = loadBitmap("font.img");

	enableSmoothMovement = TRUE;

	// that is from the N64 SDK...not our own initGL
	gl_init();

#if DEBUG_RDP
	rdpq_debug_start();
    rdpq_debug_log(true);
#endif

	initGL();
	controller_init();
}

void handleSystemEvents() {

	controller_scan();
	struct controller_data pressed = get_keys_pressed();
	struct controller_data down = get_keys_down();

	if (pressed.c[0].A) {
		mBufferedCommand = kCommandUp;
	}

	if (pressed.c[0].B) {
		mBufferedCommand = kCommandDown;
	}

	if (down.c[0].start) {
		mBufferedCommand = kCommandFire1;
	}

	if (down.c[0].R) {
	}

	if (down.c[0].C_up) {
	}


}

void graphicsShutdown() {
	releaseBitmap(defaultFont);
	texturesUsed = 0;
}

void flipRenderer() {
	gl_swap_buffers();
}
