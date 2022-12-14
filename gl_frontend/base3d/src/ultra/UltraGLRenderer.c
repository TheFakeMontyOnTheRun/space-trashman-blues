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

	enableSmoothMovement = TRUE;

	// that is from the N64 SDK...not our own initGL
	gl_init();

	initGL();
	controller_init();
}

void handleSystemEvents() {

	controller_scan();
	struct controller_data pressed = get_keys_pressed();
	struct controller_data down = get_keys_down();

	if (pressed.c[0].C_up) {
		mBufferedCommand = kCommandUp;
	}

	if (pressed.c[0].C_down) {
		mBufferedCommand = kCommandDown;
	}

	if (down.c[0].start) {
		mBufferedCommand = kCommandFire1;
	}

	if (down.c[0].A) {
		mBufferedCommand = kCommandFire1;
	}

	if (down.c[0].B) {
		mBufferedCommand = kCommandBack;
	}

	if (down.c[0].C_left) {
		mBufferedCommand = kCommandLeft;
	}

	if (down.c[0].C_right) {
		mBufferedCommand = kCommandRight;
	}


}

void graphicsShutdown() {
	texturesUsed = 0;
}

void flipRenderer() {
	gl_swap_buffers();
}
