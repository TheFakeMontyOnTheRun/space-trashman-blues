#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <stdio.h>
#include <conio.h>
#include <graphics.h>
#include <sys/ioctl.h>                // required for switching the screen mode
#include <graphics.h>
#include <games.h>
#include <psg.h>
#include <sound.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

uint8_t cursorPosition = 0;

#define BUFFER_SIZEX 16
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128
#define COOLDOWN_MAX 0x2EF
#define MARGIN_TEXT_SCREEN_LIMIT 35

uint8_t framebuffer[16*128];

uint8_t font[] = {
		// ASCII table starting on SPACE.
		// Being on line 32 is no accident.
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // space
		, 0x10, 0x38, 0x38, 0x10, 0x10, 0x00, 0x10, 0x00, 0x6c, 0x6c, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
		0x7c, 0x28, 0x28, 0x7c, 0x28, 0x00, 0x20, 0x38, 0x40, 0x30, 0x08, 0x70, 0x10, 0x00, 0x64, 0x64, 0x08, 0x10,
		0x20, 0x4c, 0x4c, 0x00, 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x00,
		0x00, 0x28, 0x38, 0x7c, 0x38, 0x28, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x30, 0x30, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00 // /space - 15
		, 0x38, 0x44, 0x4c, 0x54, 0x64, 0x44, 0x38, 0x00 // 0
		, 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x38, 0x44, 0x04, 0x18, 0x20, 0x40, 0x7c, 0x00, 0x38, 0x44,
		0x04, 0x38, 0x04, 0x44, 0x38, 0x00, 0x08, 0x18, 0x28, 0x48, 0x7c, 0x08, 0x08, 0x00, 0x7c, 0x40, 0x40, 0x78,
		0x04, 0x44, 0x38, 0x00, 0x18, 0x20, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x7c, 0x04, 0x08, 0x10, 0x20, 0x20,
		0x20, 0x00, 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00, 0x38, 0x44, 0x44, 0x3c, 0x04, 0x08, 0x30, 0x00,
		0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x20, 0x08, 0x10,
		0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04,
		0x08, 0x10, 0x20, 0x00, 0x38, 0x44, 0x04, 0x18, 0x10, 0x00, 0x10, 0x00, 0x38, 0x44, 0x5c, 0x54, 0x5c, 0x40,
		0x38, 0x00 // /0
		, 0x38, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44, 0x00 // a
		, 0x78, 0x44, 0x44, 0x78, 0x44, 0x44, 0x78, 0x00, 0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00, 0x78, 0x44,
		0x44, 0x44, 0x44, 0x44, 0x78, 0x00, 0x7c, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7c, 0x00, 0x7c, 0x40, 0x40, 0x78,
		0x40, 0x40, 0x40, 0x00, 0x38, 0x44, 0x40, 0x5c, 0x44, 0x44, 0x3c, 0x00, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44,
		0x44, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x04, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00,
		0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7c, 0x00, 0x44, 0x6c,
		0x54, 0x44, 0x44, 0x44, 0x44, 0x00, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x44, 0x44, 0x00, 0x38, 0x44, 0x44, 0x44,
		0x44, 0x44, 0x38, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40, 0x00, 0x38, 0x44, 0x44, 0x44, 0x54, 0x48,
		0x34, 0x00, 0x78, 0x44, 0x44, 0x78, 0x48, 0x44, 0x44, 0x00, 0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00,
		0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x44, 0x44,
		0x44, 0x44, 0x44, 0x28, 0x10, 0x00, 0x44, 0x44, 0x54, 0x54, 0x54, 0x54, 0x28, 0x00, 0x44, 0x44, 0x28, 0x10,
		0x28, 0x44, 0x44, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x00, 0x78, 0x08, 0x10, 0x20, 0x40, 0x40,
		0x78, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
		0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x30, 0x30, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x08,
		0x30, 0x40, 0x78, 0x00, 0x18, 0x20, 0x20, 0x60, 0x20, 0x20, 0x18, 0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x10,
		0x10, 0x00, 0x30, 0x08, 0x08, 0x0c, 0x08, 0x08, 0x30, 0x00, 0x28, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x10, 0x38, 0x6c, 0x44, 0x44, 0x7c, 0x00, 0x00
};

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

void backToGraphics();

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];
uint16_t cooldown;

void init() {
	cooldown = COOLDOWN_MAX;
	clg();
}

char *menuItems[] = {
		"8) Use/Toggle",
		"5) Use with...",
		"9) Use/pick...",
		"6) Drop",
		"7) Next item",
		"4) Next in room",
};

void graphicsFlush();

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX) {

	uint8_t len = strlen(text);
	char *ptr = text;
	uint8_t c = 0;
	uint8_t chary = 0;
	uint8_t cx;
	uint8_t cy;
	uint8_t x = _x;

	for (; c < len && y < 64; ++c) {

		char cha = *ptr;

		if (x == limitX) {
			++y;
			x = _x;
		} else if (cha == '\n') {
			++y;
			x = _x;
			++ptr;
			continue;
		}

		if (cha >= 'a') {
			if (cha <= 'z') {
				cha = (cha - 'a') + 'A';
			} else {
				cha -= ('z' - 'a');
			}
		}

		uint8_t *fontTop = &font[((cha - 32) << 3)];

		for (cy = 0; cy < 8; cy++ ) {
			uint8_t charLine = *fontTop;

			for (cx = 0; cx < 8; cx++ ) {
				if ( charLine & 1) {
					plot(((x) * 8) + (8 - cx), (y * 8) + cy);
				} else {
					unplot(((x) * 8) + (8 - cx), (y * 8) + cy);
				}
				charLine = charLine >> 1;
			}
			fontTop++;
		}

		++x;
		++ptr;
	}
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
	writeStrWithLimit(_x, y, text, MARGIN_TEXT_SCREEN_LIMIT);
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {}


void showMessage(const char *message) {
	uint8_t keepGoing = 1;
	clearScreen();

	writeStr(1, 1, message, 2, 0);
	writeStr(2, 22, "Press SPACE button to continue", 2, 0);

	while (keepGoing) {
		if (getKey() == ' ') {
			keepGoing = 0;
		}
	}

	backToGraphics();
}

void titleScreen() {
	uint8_t keepGoing = 1;
	clearScreen();

	writeStr(1, 1, "Space Mare Imperium: Derelict", 2, 0);
	writeStr(1, 4, "by Daniel Monteiro", 2, 0);
	writeStr(1, 6, " Press SPACE to start ", 2, 0);

	while (keepGoing) {
		if (getKey() == ' ') {
			keepGoing = 0;
		}
	}
	backToGraphics();
}

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

void performAction(void) {
	switch (getGameStatus()) {
		case kBadVictory:
			showMessage("Victory! Too bad you didn't survive");
			while (1);

		case kBadGameOver:
			showMessage("You're dead! And so are the\n"
						"other people on the path of\n"
						"destruction faulty reactor");
			while (1);

		case kGoodVictory:
			showMessage("Victory! You managed to destroy the\nship and get out alive");
			while (1);

		case kGoodGameOver:
			showMessage("You failed! While you're alive\n"
						"you failed to prevent the worst\n"
						"scenario and now EVERYBODY is\n"
						"dead!)");
			while (1);

		default:
		case kNormalGameplay:
			break;
	}
}

void clearTextScreen() {
	clearScreen();
}

void enterTextMode() {
}

void exitTextMode() {
}

uint8_t getKey() {
	uint8_t input = getch();

	performAction();

	switch (input) {
		case 30:
			return 'w';
		case 31:
			return 's';
		case 29:
			return 'q';
		case 28:
			return 'e';
		case 'z':
			return 'a';

		case 'x':
			return 'd';

	}
	return input;
}

void shutdownGraphics() {
}

void clearScreen() {
	clg();
}

void clearGraphics() {
	memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void graphicsFlush() {

	for (int y = 0; y < BUFFER_SIZEY; ++y ) {
		for (int x = 0; x < BUFFER_SIZEX; ++x ) {
			uint8_t chunk = buffer[(y * BUFFER_SIZEX) + x];

			if (chunk & 1 ) {
				plot( (x * 8), y );
			} else {
				unplot( (x * 8), y );
			}

			if (chunk & 2 ) {
				plot( (x * 8) + 1, y );
			} else {
				unplot( (x * 8) + 1, y );
			}

			if (chunk & 4 ) {
				plot( (x * 8) + 2, y );
			} else {
				unplot( (x * 8) + 2, y );
			}

			if (chunk & 8 ) {
				plot( (x * 8) + 3, y );
			} else {
				unplot( (x * 8) + 3, y );
			}

			if (chunk & 16 ) {
				plot( (x * 8) + 4, y );
			} else {
				unplot( (x * 8) + 4, y );
			}

			if (chunk & 32 ) {
				plot( (x * 8) + 5, y );
			} else {
				unplot( (x * 8) + 5, y );
			}

			if (chunk & 64 ) {
				plot( (x * 8) + 6, y );
			} else {
				unplot( (x * 8) + 6, y );
			}

			if (chunk & 128 ) {
				plot( (x * 8) + 7, y );
			} else {
				unplot( (x * 8) + 7, y );
			}
		}
	}
}

void sleepForMS(uint32_t ms) {
}


void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

	uint8_t _y0 = y0;
	uint8_t _y1 = y1;

	if (y0 > y1) {
		_y0 = y1;
		_y1 = y0;
	}

	uint8_t *ptr = &buffer[(_y0 * (BUFFER_SIZEX)) + (x0 / 8)]; //skip to the line in pattern
	uint8_t y;
	switch (x0 & 7) {
		case 0:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 128;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
		case 1:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 64;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
		case 2:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 32;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
		case 3:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 16;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
		case 4:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 8;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
		case 5:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 4;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
		case 6:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 2;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
		case 7:
			for (y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 1;
				}
				ptr += BUFFER_SIZEX;
			}
			break;
	}
}


uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {
	ptr = &buffer[(y * (BUFFER_SIZEX)) + (x / 8)]; //skip to the line in pattern

	switch (x & 7) {
		case 7:
			*ptr |= 128;
			break;
		case 6:
			*ptr |= 64;
			break;
		case 5:
			*ptr |= 32;
			break;
		case 4:
			*ptr |= 16;
			break;
		case 3:
			*ptr |= 8;
			break;
		case 2:
			*ptr |= 4;
			break;
		case 1:
			*ptr |= 2;
			break;
		case 0:
			*ptr |= 1;
			break;
	}

	return ptr;
}

void graphicsPut(uint8_t x, uint8_t y) {

	uint8_t *ptr = &buffer[(y * (BUFFER_SIZEX)) + (x / 8)]; //skip to the line in pattern

	switch (x & 7) {
		case 7:
			*ptr |= 128;
			break;
		case 6:
			*ptr |= 64;
			break;
		case 5:
			*ptr |= 32;
			break;
		case 4:
			*ptr |= 16;
			break;
		case 3:
			*ptr |= 8;
			break;
		case 2:
			*ptr |= 4;
			break;
		case 1:
			*ptr |= 2;
			break;
		case 0:
			*ptr |= 1;
			break;
	}
}

void HUD_initialPaint() {
	struct Room *room = getRoom(getPlayerRoom());

	for (uint8_t i = 0; i < 6; ++i) {
		writeStr(17, 14 + i, menuItems[i], 2, 0);
	}

	HUD_refresh();
}

void HUD_refresh() {

	writeStrWithLimit(17, 5, "Object in hand", MARGIN_TEXT_SCREEN_LIMIT);

	if (focusedItem != NULL) {
		struct Item *item = getItem(focusedItem->item);


		if (item->active) {
			writeStr(17, 6, "*", 2, 0);
		}

		writeStrWithLimit(17, 6, item->name, MARGIN_TEXT_SCREEN_LIMIT);
	} else {
		writeStrWithLimit(17, 6, "Nothing", MARGIN_TEXT_SCREEN_LIMIT);
	}

	writeStrWithLimit(17, 1, "Object in room", MARGIN_TEXT_SCREEN_LIMIT);

	if (roomItem != NULL) {
		struct Item *item = getItem(roomItem->item);


		if (item->active) {
			writeStrWithLimit(17, 2, "*", MARGIN_TEXT_SCREEN_LIMIT);
		}

		writeStrWithLimit(17, 2, item->name, MARGIN_TEXT_SCREEN_LIMIT);
	} else {
		writeStrWithLimit(17, 2, "Nothing", MARGIN_TEXT_SCREEN_LIMIT);
	}
}

