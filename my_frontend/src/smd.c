#include <genesis.h>
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

#define COOLDOWN_MAX 0x2EF

uint8_t buffered = '.';
uint8_t cursorPosition = 0;
uint16_t cooldown;
uint16_t movementCooldown = 0;

char *menuItems[] = {
		"Use/Toggle",
		"Use with...",
		"Use/pick...",
		"Drop",
		"Next item",
		"Next in room",
};

void refreshJustGraphics() {
	clearGraphics();
	renderScene();
	graphicsFlush();
}

void backToGraphics() {
	clearScreen();
	HUD_initialPaint();
	refreshJustGraphics();
}

void performAction() {
	struct Room *room = getRoom(getPlayerRoom());

	switch (getGameStatus()) {
		case kBadVictory:
			showMessage("Victory! Too bad you didn't survive\nto tell the story\n\n\n\n\n\n");
			while (1);

		case kBadGameOver:
			showMessage("You're dead! And so are millions of\n"
						"other people on the path of\n"
						"destruction faulty reactor\n\n\n\n\n\n");
			while (1);

		case kGoodVictory:
			showMessage("Victory! You managed to destroy the\nship and get out alive\n\n\n\n\n\n");
			while (1);

		case kGoodGameOver:
			showMessage("You failed! While you fled the ship\n"
						"alive, you failed to prevent the \n"
						"worstscenario and now EVERYBODY is\n"
						"dead (and that includes you!)\n\n\n\n\n");
			while (1);

		default:
		case kNormalGameplay:
			break;
	}

/*
char *menuItems[] = {
 0       "Use/Toggle current item",
 1       "Use current item with...",
 2       "Pick",
 3       "Drop",
 4       "Next item in inventory",
 5       "Next room item in focus",
};
*/

	switch (cursorPosition) {
		case 0:
			useObjectNamed(getItem(focusedItem->item)->name);
			break;
		case 1:
			interactWithItemInRoom();
			HUD_refresh();
			break;
		case 2:
			pickItem();
			refreshJustGraphics();
			HUD_refresh();
			break;
		case 3:
			dropItem();
			refreshJustGraphics();
			HUD_refresh();
			break;
		case 4:
			nextItemInHand();
			break;
		case 5:
			nextItemInRoom();
			break;
	}


}


static void handleInput() {
	u16 value;
	buffered = '.';

	if (cooldown) {
		cooldown--;
	}

	if (movementCooldown) {
		movementCooldown--;
	}

	// need to call it manually as we don't use SYS_doVBlankProcess() here
	JOY_update();

	value = JOY_readJoypad(JOY_1);

	if (value & BUTTON_A) {
		if (value & BUTTON_LEFT) {
			buffered = 'a';
		} else if (value & BUTTON_RIGHT) {
			buffered = 'd';
		}
	} else {

		if (value & BUTTON_UP && !movementCooldown) {
			buffered = 'w';
		}

		if (value & BUTTON_DOWN && !movementCooldown) {
			buffered = 's';
		}

		if (value & BUTTON_LEFT && !movementCooldown) {
			buffered = 'q';
		}

		if (value & BUTTON_RIGHT && !movementCooldown) {
			buffered = 'e';
		}

		if (value & BUTTON_START) {
			buffered = 'k';
		}

		if ((value & BUTTON_B) && !cooldown) {
			performAction();
			HUD_refresh();
			cooldown = COOLDOWN_MAX;
			buffered = 'p';
		}

		if ((value & BUTTON_C) && !cooldown) {
			cursorPosition = (cursorPosition + 1);

			if (cursorPosition >= 6) {
				cursorPosition = 0;
			}

			HUD_refresh();
			cooldown = COOLDOWN_MAX;
			buffered = 'p';
		}
	}
}

void shutdownGraphics() {
}


void writeStrWithLimit(uint8_t _x, uint8_t y, char *str, uint8_t limitX) {

	char textBuffer[2];
	char *charPtr = &textBuffer[0];

	uint8_t len = strlen(str);
	uint8_t x = _x;
	textBuffer[1] = 0;

	for (uint8_t c = 0; c < len && y < 19; ++c) {

		char cha = *str;

		if (x == limitX) {
			++y;
			x = _x;
		} else if (cha == '\n') {
			++y;
			x = _x;
			++str;
			continue;
		}

		*charPtr = ' ';
		VDP_drawText(charPtr, x, y);
		*charPtr = cha;
		VDP_drawText(charPtr, x, y);
		++x;
		++str;
	}
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
	writeStrWithLimit(_x, y, text, 31);
}

void graphicsPut(uint8_t x, uint8_t y) {
	BMP_setPixelFast(x, 16 + y, 0b11101110);
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

	uint8_t stipple;

	if (y0 > y1) {
		uint8_t tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	uint8_t colour;

	if (shouldStipple <= 7) {
		colour = shouldStipple;
		shouldStipple = 0;
		stipple = 1;
	} else {
		colour = shouldStipple - 8;
		shouldStipple = 1;
		stipple = (x0 & 1);
	}

	colour += (colour << 4); //double the pixel

	for (uint8_t y = y0; y < y1; ++y) {
		if (shouldStipple) {
			stipple = !stipple;
		}

		if (stipple) {
			BMP_setPixelFast(x0, 16 + y, colour);
		}
	}
}

static void joyEvent(u16 joy, u16 changed, u16 state) {

}

void showMessage(const char *message) {
	enterTextMode();
	uint8_t keepGoing = 1;

	for (uint8_t i = 0; i < 19; ++i) {
		VDP_clearText(16, i, 16);
	}

	writeStrWithLimit(1, 5, message, 31);
	writeStrWithLimit(6, 17, "Press Start to continue", 31);

	while (keepGoing) {
		if (getKey() == 'k') {
			keepGoing = 0;
		}
	}

	clearScreen();

	for (uint8_t i = 0; i < 19; ++i) {
		VDP_clearText(16, i, 16);
	}

	exitTextMode();
	backToGraphics();
}

void clearScreen() {
}

void clearGraphics() {
	BMP_clear();
}

void clearTextScreen() {
	for (uint8_t c = 0; c < 23; ++c) {
		VDP_clearText(0, c, 256 / 8);
	}
	cooldown = COOLDOWN_MAX;
	movementCooldown = COOLDOWN_MAX;
}

void enterTextMode() {
	clearGraphics();
	BMP_flip(1);
	clearTextScreen();
}

void exitTextMode() {
	clearTextScreen();
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {}

uint8_t getKey() {
	handleInput();
	return buffered;
}

void sleepForMS(uint32_t ms) {
	//we cant afford to sleep
}

void titleScreen() {
	uint8_t keepGoing = 1;
	clearGraphics();
	//               |
	writeStr(1, 5, "   Space Mare Imperium  ", 2, 0);
	writeStr(1, 6, "        Derelict        ", 2, 0);
	writeStr(1, 8, "   by Daniel Monteiro   ", 2, 0);
	writeStr(1, 10, "   Press start button!  ", 2, 0);

	while (keepGoing) {
		if (getKey() == 'k') {
			keepGoing = 0;
		}
	}


	VDP_clearText(1, 5, 24);
	VDP_clearText(1, 6, 24);
	VDP_clearText(1, 8, 24);
	VDP_clearText(1, 10, 24);

	clearScreen();
}

void puts(char *unused) {

}

void assert(int unused) {

}

void init() {
	JOY_setEventHandler(joyEvent);
	VDP_setScreenWidth256();
	VDP_setHInterrupt(0);
	VDP_setHilightShadow(0);

	DMA_setBufferSize(2048);

	BMP_init(FALSE, BG_B, PAL0, 0);

	PAL_setColor( 0, RGB24_TO_VDPCOLOR(0x000000));
	PAL_setColor( 1, RGB24_TO_VDPCOLOR(0x0000ff));
	PAL_setColor( 2, RGB24_TO_VDPCOLOR(0x00ff00));
	PAL_setColor( 3, RGB24_TO_VDPCOLOR(0x00ffff));
	PAL_setColor( 4, RGB24_TO_VDPCOLOR(0xff0000));
	PAL_setColor( 5, RGB24_TO_VDPCOLOR(0xff00ff));
	PAL_setColor( 6, RGB24_TO_VDPCOLOR(0xffff00));
	PAL_setColor( 7, RGB24_TO_VDPCOLOR(0xffffff));
	PAL_setColor( 8, RGB24_TO_VDPCOLOR(0x000000));
	PAL_setColor( 9, RGB24_TO_VDPCOLOR(0x00007f));
	PAL_setColor(10, RGB24_TO_VDPCOLOR(0x007f00));
	PAL_setColor(11, RGB24_TO_VDPCOLOR(0x007f7f));
	PAL_setColor(12, RGB24_TO_VDPCOLOR(0x7f0000));
	PAL_setColor(13, RGB24_TO_VDPCOLOR(0x7f007f));
	PAL_setColor(14, RGB24_TO_VDPCOLOR(0x7f7f00));
	PAL_setColor(15, RGB24_TO_VDPCOLOR(0x7f7f7f));
}

void graphicsFlush() {
	HUD_initialPaint();
	BMP_flip(1);
	BMP_clear();
}

void HUD_initialPaint() {
	struct Room *room = getRoom(getPlayerRoom());


	for (uint8_t i = 0; i < 6; ++i) {
		writeStr(16, 13 + i, i == cursorPosition ? ">" : " ", 2, 0);
		writeStr(17, 13 + i, menuItems[i], 2, 0);
	}

	HUD_refresh();
}

void HUD_refresh() {

	for (uint8_t i = 0; i < 13; ++i) {
		VDP_clearText(16, i, 16);
	}

	for (uint8_t i = 0; i < 6; ++i) {
		writeStr(16, 13 + i, (i == cursorPosition) ? ">" : " ", 2, 0);
	}


	writeStrWithLimit(16, 5, "Object in hand:", 31);
	if (focusedItem != NULL) {
		struct Item *item = getItem(focusedItem->item);


		if (item->active) {
			writeStr(16, 6, "*", 2, 0);
		}

		writeStrWithLimit(17, 6, item->name, 31);
	} else {
		writeStrWithLimit(16, 6, "Nothing", 31);
	}

	writeStrWithLimit(16, 8, "Object in room:", 31);

	if (roomItem != NULL) {
		struct Item *item = getItem(roomItem->item);


		if (item->active) {
			writeStrWithLimit(16, 9, "*", 31);
		}

		writeStrWithLimit(17, 9, item->name, 31);
	} else {
		writeStrWithLimit(16, 9, "Nothing", 31);
	}
}