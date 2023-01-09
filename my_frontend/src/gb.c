#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <gb/gb.h>
#include <gb/drawing.h>


#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"


#define COOLDOWN_MAX 0x2EF
#define MARGIN_TEXT_SCREEN_LIMIT 30

extern const struct Pattern patterns[96];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

void clearGraphics(void);

void backToGraphics(void);

uint8_t cursorPosition = 0;

void init(void) {
	OBP1_REG = 0xE0U;
}

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX) {
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {}

void showMessage(const char *message) {

}

void titleScreen(void) {
}

void performAction(void) {
}

void clearTextScreen(void) {
}

void enterTextMode(void) {
}

void exitTextMode(void) {
}

uint8_t getKey(void) {

	int key = joypad();
	if(key & (J_UP|J_DOWN|J_LEFT|J_RIGHT)) {
		box( 0, 0, 128, 128, M_FILL);
		if(key & J_UP) {
			return 'w';
		}

		if(key & J_DOWN) {
			return 's';
		}

		if(key & J_LEFT) {
			return 'q';
		}

		if(key & J_RIGHT) {
			return 'e';
		}
	}
//	unsigned int key = read_joypad1();
//
//	if (cooldown) {
//		cooldown--;
//	}
//
//	if (key & JOY_UP) {
//		return 'w';
//	}
//
//	if (key & JOY_LEFT) {
//		if (key & JOY_FIREB) {
//			return 'a';
//		} else {
//			return 'q';
//		}
//	}
//
//	if (key & JOY_RIGHT) {
//		if (key & JOY_FIREB) {
//			return 'd';
//		} else {
//			return 'e';
//		}
//	}
//
//	if (key & JOY_DOWN) {
//		return 's';
//	}
//
//	if ((key & JOY_FIREA) && !cooldown) {
//		performAction();
//		cooldown = COOLDOWN_MAX;
//		return 'p';
//	}
//
//	if ((key & JOY_FIREB) && !cooldown) {
//		cursorPosition = (cursorPosition + 1);
//
//		if (cursorPosition >= 6) {
//			cursorPosition = 0;
//		}
//
//		HUD_refresh();
//		cooldown = COOLDOWN_MAX;
//		return 'p';
//	}

	return '.';
}

void shutdownGraphics(void) {
}

void clearScreen(void) {

}

void clearGraphics(void) {

}

void graphicsFlush(void) {
	wait_vbl_done();
}

void sleepForMS(uint32_t ms) {
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {
	line(x0, y0, x0, y1 );
}

void graphicsPut(uint8_t x, uint8_t y) {
	plot(x, y, BLACK, SOLID);
}


void HUD_initialPaint(void) {
}

void HUD_refresh(void) {
}

