#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sms.h>
#include <stdio.h>
#include <msx/gfx.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern int accessGrantedToSafe;

#define BUFFER_SIZEX 16
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128
#define COOLDOWN_MAX 0x2EF
#define MARGIN_TEXT_SCREEN_LIMIT 30

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

int cursorPosition = 0;

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];
uint16_t cooldown;

void init() {
	set_color(15, 1, 1);
	set_mode(mode_2);
	fill(MODE2_ATTR, 0xF1, MODE2_MAX);
	cooldown = COOLDOWN_MAX;
}

char *menuItems[] = {
		"Use/Toggle",
		"Use with...",
		"Use/pick...",
		"Drop",
		"Next item",
		"Next in room",
};

void graphicsFlush();

void writeStrWithLimit(int _x, int y, char *text, int limitX) {

	uint8_t len = strlen(text);
	char *ptr = text;
	uint8_t c = 0;
	uint8_t chary = 0;
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

		uint8_t baseY = (y << 3);

		uint8_t *fontTop = &font[((cha - 32) << 3)];
		vwrite(fontTop, map_pixel(x << 3, y << 3), 8);

		++x;
		++ptr;
	}
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
	writeStrWithLimit(_x, y, text, MARGIN_TEXT_SCREEN_LIMIT);
}

void drawWindow(int tx, int ty, int tw, int th, const char *title) {}


void showMessage(const char *message) {
	int keepGoing = 1;
	clearScreen();

	writeStr(1, 1, message, 2, 0);
	writeStr(2, 22, "Press any button to continue", 2, 0);

	while (keepGoing) {
		if (getKey() == 'p') {
			keepGoing = 0;
		}
	}

	clearScreen();
	HUD_initialPaint();
	renderScene();
}

void titleScreen() {
	int keepGoing = 1;
	clearGraphics();

	writeStr(1, 1, "Space Mare Imperium: Derelict", 2, 0);
	writeStr(1, 4, "by Daniel Monteiro", 2, 0);
	writeStr(1, 6, "  Press B to start ", 2, 0);

	while (keepGoing) {
		if (getKey() == 'p') {
			keepGoing = 0;
		}
	}

	clearScreen();
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
						"dead!)\n\n\n\n\n");
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
			break;
		case 2:
			pickItem();
			clearGraphics();
			renderScene();
			graphicsFlush();
			HUD_refresh();
			break;
		case 3:
			dropItem();
			clearGraphics();
			renderScene();
			graphicsFlush();
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

void clearTextScreen() {
	clearScreen();
}

void enterTextMode() {
}

void exitTextMode() {
}

uint8_t getKey() {
	unsigned int key = read_joypad1();

	if (cooldown) {
		cooldown--;
	}

	if (key & JOY_UP) {
		return 'w';
	}

	if (key & JOY_LEFT) {
		if (key & JOY_FIREB) {
			return 'a';
		} else {
			return 'q';
		}
	}


	if (key & JOY_RIGHT) {
		if (key & JOY_FIREB) {
			return 'd';
		} else {
			return 'e';
		}
	}

	if (key & JOY_DOWN) {
		return 's';
	}

	if ((key & JOY_FIREA) && !cooldown) {
		performAction();
		cooldown = COOLDOWN_MAX;
		HUD_refresh();
		return 'p';
	}

	if ((key & JOY_FIREB) && !cooldown) {
		cursorPosition = (cursorPosition + 1);

		if (cursorPosition >= 6) {
			cursorPosition = 0;
		}

		HUD_refresh();
		cooldown = COOLDOWN_MAX;
		return 'p';
	}

	return '.';
}

void shutdownGraphics() {
}

void clearScreen() {
	set_mode(mode_2);
	fill(MODE2_ATTR, 0xF1, MODE2_MAX);
}

void clearGraphics() {
	memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void graphicsFlush() {
	uint8_t *ptr = &buffer[0];
	for (uint8_t y = 0; y < BUFFER_RESY; y += 8) {
		uint16_t addr = map_pixel(0, y + 32);
		vwrite(ptr, addr, 16 * 8);
		ptr += 8 * 16;
	}
}

void sleepForMS(uint32_t ms) {
	//we cant afford to sleep
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

#ifdef HALF_BUFFER
	x0 = x0 >> 1;
	y0 = y0 >> 1;
	y1 = y1 >> 1;
#endif

	uint8_t _y0 = y0;
	uint8_t _y1 = y1;


	if (x0 >= BUFFER_RESX) return;

	if (y0 > y1) {
		_y0 = y1;
		_y1 = y0;
	}

	if (_y1 >= BUFFER_RESY) {
		_y1 = BUFFER_RESY - 1;
	};


	if (_y0 >= BUFFER_RESY) {
		_y0 = BUFFER_RESY - 1;
	};

	uint8_t patternLine = (_y0 & 7); //which line inside the pattern;
	uint8_t *ptr = &buffer[(16 * 8 * (_y0 >> 3)) + //skip the entire row of patterns along the y
						   (8 * (x0 >> 3)) + //skip to the correct pattern in the row
						   patternLine]; //skip to the line in pattern

	switch (x0 & 7) {
		case 0:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 128;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}
			break;
		case 1:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 64;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}
			break;
		case 2:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 32;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}
			break;
		case 3:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 16;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}
			break;
		case 4:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 8;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}

			break;
		case 5:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 4;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}

			break;
		case 6:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 2;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}

			break;
		case 7:
			for (uint8_t y = _y0; y <= _y1; ++y) {
				if (!shouldStipple || (y & 1)) {
					*ptr |= 1;
				}

				++patternLine;
				++ptr;

				if (patternLine >= 8) {
					patternLine = 0;
					ptr += (16 * 8) - 8;
				}
			}

			break;
	}
}


uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {

#ifdef HALF_BUFFER
	x = x >> 1;
	y = y >> 1;
#endif

	if (ptr == NULL) {
		if (y >= BUFFER_RESY) return NULL;

		ptr = &buffer[(16 * 8 * (y >> 3)) + //skip the entire row of patterns along the y
					  (8 * (x >> 3)) + //skip to the correct pattern in the row
					  (y & 7)]; //skip to the line in pattern
	}

	switch (x & 7) {
		case 0:
			*ptr |= 128;
			break;
		case 1:
			*ptr |= 64;
			break;
		case 2:
			*ptr |= 32;
			break;
		case 3:
			*ptr |= 16;
			break;
		case 4:
			*ptr |= 8;
			break;
		case 5:
			*ptr |= 4;
			break;
		case 6:
			*ptr |= 2;
			break;
		case 7:
			*ptr |= 1;
			break;
	}

	return ptr;
}

void graphicsPut(uint8_t x, uint8_t y) {

#ifdef HALF_BUFFER
	x = x >> 1;
	y = y >> 1;
#endif

	if (y >= BUFFER_RESY || x >= BUFFER_RESX) return;

	uint8_t *ptr = &buffer[(16 * 8 * (y >> 3)) + //skip the entire row of patterns along the y
						   (8 * (x >> 3)) + //skip to the correct pattern in the row
						   (y & 7)]; //skip to the line in pattern

	switch (x & 7) {
		case 0:
			*ptr |= 128;
			break;
		case 1:
			*ptr |= 64;
			break;
		case 2:
			*ptr |= 32;
			break;
		case 3:
			*ptr |= 16;
			break;
		case 4:
			*ptr |= 8;
			break;
		case 5:
			*ptr |= 4;
			break;
		case 6:
			*ptr |= 2;
			break;
		case 7:
			*ptr |= 1;
			break;
	}
}


void HUD_initialPaint() {
	struct Room *room = getRoom(getPlayerRoom());

	draw(BUFFER_RESX, 0, BUFFER_RESX, 191);

	for (uint8_t i = 0; i < 6; ++i) {
		writeStr(18, 14 + i, menuItems[i], 2, 0);
	}

	HUD_refresh();
}

void HUD_refresh() {

	for (int c = 0; c < 13; ++c) {
		for (int d = 0; d < 15; ++d) {
			writeStr(17 + d, c, " ", 2, 0);
		}
	}

	for (uint8_t i = 0; i < 6; ++i) {
		writeStr(17, 14 + i, (i == cursorPosition) ? ">" : " ", 2, 0);
	}

	writeStrWithLimit(17, 5, "Object in hand", 31);

	if (focusedItem != NULL) {
		struct Item *item = getItem(focusedItem->item);


		if (item->active) {
			writeStr(17, 6, "*", 2, 0);
		}

		writeStrWithLimit(18, 6, item->name, 31);
	} else {
		writeStrWithLimit(18, 6, "Nothing", 31);
	}

	writeStrWithLimit(17, 1, "Object in room", 31);

	if (roomItem != NULL) {
		struct Item *item = getItem(roomItem->item);


		if (item->active) {
			writeStrWithLimit(17, 2, "*", 31);
		}

		writeStrWithLimit(18, 2, item->name, 31);
	} else {
		writeStrWithLimit(18, 2, "Nothing", 31);
	}
}

