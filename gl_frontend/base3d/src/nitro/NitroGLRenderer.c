#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <nds.h>
#include <malloc.h>
#include <nds/arm9/image.h>
#include <nds/arm9/trig_lut.h>
#include <filesystem.h>

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

#define COOLDOWN 0x10
int snapshotSignal = '.';

void graphicsInit() {
	videoSetMode(MODE_0_3D);
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_TEXTURE);
	glInit();

	enableSmoothMovement = TRUE;
	initGL();
	defaultFont = loadBitmap("font.img");
}

int cooldown = 0;
void handleSystemEvents() {
	scanKeys();

	if (cooldown > 0) {
		cooldown--;
	} else {

		u16 keys = keysHeld();

		if((keys & KEY_UP)) {
			cooldown = COOLDOWN;
			mBufferedCommand = kCommandUp;
		}
		if((keys & KEY_DOWN)) {
			cooldown = COOLDOWN;
			mBufferedCommand = kCommandDown;
		}

		if((keys & KEY_LEFT)) {
			cooldown = COOLDOWN;
			mBufferedCommand = kCommandLeft;
		}
		if((keys & KEY_RIGHT)) {
			cooldown = COOLDOWN;
			mBufferedCommand = kCommandRight;
		}
		if((keys & KEY_START)) {
			cooldown = COOLDOWN;
			mBufferedCommand = kCommandFire1;
		}
	}

}

void graphicsShutdown() {
	texturesUsed = 0;
}

void flipRenderer() {
	swiWaitForVBlank();
}
