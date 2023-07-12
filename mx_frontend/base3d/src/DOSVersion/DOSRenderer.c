#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "Enums.h"
#include "FixP.h"
#include "Common.h"
#include "Vec.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "PackedFileReader.h"
#include "Vec.h"

#include "CTile3DProperties.h"

#include "Core.h"
#include "Engine.h"
#include "LoadBitmap.h"
#include "CRenderer.h"
#include "Globals.h"

typedef int ESoundDriver;

extern ESoundDriver soundDriver;

uint8_t turnBuffer[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];

enum EColor {
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_BROWN,
	COLOR_LIGHT_GRAY,
	COLOR_DARK_GRAY,
	COLOR_LIGHT_BLUE,
	COLOR_LIGHT_GREEN,
	COLOR_LIGHT_CYAN,
	COLOR_LIGHT_RED,
	COLOR_LIGHT_MAGENTA,
	COLOR_YELLOW,
	COLOR_WHITE,
};

long frame = 0;

void graphicsShutdown() {
	texturesUsed = 0;

	textmode(C80);
	clrscr();
	printf(
			"Thanks for playing!\nDOS is back with a vengeance!.\n\n");
}

void putStr(int x, int y, const char *str, int fg, int bg) {
	int col = x;
	int row = y;
	int currAttrib = (bg << 4) | fg;
	size_t len = strlen(str);

	for (int ch = 0; ch < len; ++ch) {
		char c = str[ch];
		++col;
		_farpokeb(_dos_ds, 0xB800 * 16 + ((80 * row * 2) + (col * 2)), c);
		_farpokeb(_dos_ds, 0xB800 * 16 + ((80 * row * 2) + (col * 2)) + 1,
				  currAttrib);
	}
}

void drawTitleBox() {

	putStr(1, 2, "\xc9", COLOR_WHITE, COLOR_BLUE);

	for (int c = 0; c < 75; ++c) {
		putStr(c + 2, 2, "\xcd", COLOR_WHITE, COLOR_BLUE);
	}
	putStr(76, 2, "\xbb", COLOR_WHITE, COLOR_BLUE);

	for (int d = 3; d < 10; ++d) {
		putStr(1, d, "\xba", COLOR_WHITE, COLOR_BLUE);

		for (int c = 0; c < 75; ++c) {

			putStr(c + 2, d, " ", COLOR_WHITE, COLOR_BLUE);
		}
		putStr(76, d, "\xba", COLOR_WHITE, COLOR_BLUE);
	}

	putStr(1, 10, "\xc8", COLOR_WHITE, COLOR_BLUE);

	for (int c = 0; c < 75; ++c) {
		putStr(c + 2, 10, "\xcd", COLOR_WHITE, COLOR_BLUE);
	}
	putStr(76, 10, "\xbc", COLOR_WHITE, COLOR_BLUE);

	putStr(17, 4, "Sub Mare Imperium - Derelict - v0.9", COLOR_WHITE,
		   COLOR_BLUE);

	putStr(5, 6, "Program and Audio-visual (C) 2021 by Brotherhood of 13h",
		   COLOR_WHITE, COLOR_BLUE);

	putStr(30, 7, "PRE-RELEASE version! Beware!", COLOR_WHITE, COLOR_BLUE);
}

void querySoundDriver() {
	putStr(30, 14, "Select Sound Driver:", COLOR_YELLOW, COLOR_BLACK);
	putStr(27, 16, "1) IBM Sound", COLOR_YELLOW, COLOR_BLACK);
	putStr(27, 17, "2) Ad lib Sound Board", COLOR_YELLOW, COLOR_BLACK);
	putStr(27, 18, "3) OPL2LPT on LPT1", COLOR_YELLOW, COLOR_BLACK);
	putStr(27, 19, "4) No Sound", COLOR_YELLOW, COLOR_BLACK);
	int option = -1;

	while (option == -1) {

		gotoxy(53, 15);
		option = getch();

		switch (option) {
			case '1':
				/*				soundDriver = ESoundDriver::kPcSpeaker; */
				soundDriver = kPcSpeaker;
				return;
			case '2':
				/*				soundDriver = ESoundDriver::kAdlib; */
				soundDriver = kAdlib;
				setupOPL2(0x0388);
				return;
			case '3':
				/*				soundDriver = ESoundDriver::kOpl2Lpt; */
				soundDriver = kOpl2Lpt;
				setupOPL2(-1);
				return;
			case '4':
				return;
			default:
				option = -1;
		}
	}
}

uint8_t getPaletteEntry(uint32_t origin) {
	uint8_t shade;

	if (!(origin & 0xFF000000)) {
		return TRANSPARENCY_COLOR;
	}

	shade = 0;
	shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
	shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
	shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

	return shade;
}

void graphicsInit() {
	textmode(C80);
	clrscr();
	drawTitleBox();
	querySoundDriver();

	__dpmi_regs reg;

	reg.x.ax = 0x13;
	__dpmi_int(0x10, &reg);

	outp(0x03c8, 0);

	for (int r = 0; r < 4; ++r) {
		for (int g = 0; g < 8; ++g) {
			for (int b = 0; b < 8; ++b) {
				outp(0x03c9, (r * (16)));
				outp(0x03c9, (g * (8)));
				outp(0x03c9, (b * (8)));
			}
		}
	}

	defaultFont = loadBitmap("font.img");
	enableSmoothMovement = TRUE;
}

void handleSystemEvents() {

	int lastKey = 0;

	if (kbhit()) {
		char getched = getch();
		switch (getched) {

			case 27:
			case 'q':
				mBufferedCommand = kCommandBack;
				break;

			case '1':
				enableSmoothMovement = TRUE;
				break;

			case '2':
				enableSmoothMovement = FALSE;
				break;

			case 's':
				mBufferedCommand = kCommandStrafeLeft;
				visibilityCached = FALSE;
				break;
			case 'd':
				mBufferedCommand = kCommandStrafeRight;
				visibilityCached = FALSE;
				break;

			case 'b':
				visibilityCached = FALSE;
				break;
			case 'v':
				mBufferedCommand = kCommandFire4;
				break;
			case 'j':
				visibilityCached = FALSE;
				break;
			case 'k':
				visibilityCached = FALSE;
				break;
#ifdef SDLSW
			case 'i':
				mSlow = FALSE;
				visibilityCached = FALSE;
				break;
			case 'o':
				mSlow = TRUE;
				visibilityCached = FALSE;
				break;
#endif
			case 13:
			case 'z':
				mBufferedCommand = kCommandFire1;
				needsToRedrawVisibleMeshes = TRUE;
				visibilityCached = FALSE;
				break;

			case 'x':
			case ' ':
				mBufferedCommand = kCommandFire2;
				needsToRedrawVisibleMeshes = TRUE;
				visibilityCached = FALSE;
				break;

			case '\t':
			case 'c':
				mBufferedCommand = kCommandFire3;
				visibilityCached = FALSE;
				break;


			case 224:
			case 0: {
				char arrow = getch();
				switch (arrow) {
					case 75:
						mBufferedCommand = kCommandLeft;
						if ((currentGameMenuState == kPlayGame ||
							 currentGameMenuState == kBackToGame) &&
							currentPresentationState == kWaitingForInput
								) {
							turnStep = 0;
							turnTarget = 200;
						}
						visibilityCached = FALSE;
						break;
					case 72:
						mBufferedCommand = kCommandUp;
						visibilityCached = FALSE;
						break;
					case 77:
						mBufferedCommand = kCommandRight;
						if ((currentGameMenuState == kPlayGame ||
							 currentGameMenuState == kBackToGame) &&
							currentPresentationState == kWaitingForInput
								) {
							turnStep = 200;
							turnTarget = 0;
						}
						visibilityCached = FALSE;
						break;
					case 80:
						mBufferedCommand = kCommandDown;
						visibilityCached = FALSE;
						break;
				}
			}
				break;
		}

        if (mBufferedCommand != kCommandLeft && mBufferedCommand != kCommandRight && mBufferedCommand != kCommandNone) {
            turnStep = 0;
            turnTarget = 0;
        }
	}
}

void waitVSync() {
    /*
	DrawWait:
	asm
	{
	mov dx, 0x03da
	in al, dx
	test al, 00001000b
	jz DrawWait
	}
     */
}

void flipRenderer() {
    renderPageFlip(&turnBuffer[0], framebuffer,
                   previousFrame, turnStep, turnTarget, 0);
    dosmemput(&turnBuffer[0], XRES_FRAMEBUFFER * YRES_FRAMEBUFFER, 0xa0000);
}

void clear() {}
