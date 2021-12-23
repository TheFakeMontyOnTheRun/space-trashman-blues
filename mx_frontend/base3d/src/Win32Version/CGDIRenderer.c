
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#ifdef MSVC
#include "resource.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Win32Int.h"

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

#include "Engine.h"
#include "LoadBitmap.h"
#include "CRenderer.h"


HACCEL hAccelTable;
int needsRedraw = 1;
RGBQUAD paletteRef[256];
int havePalette = 0;
uint8_t slideBitmap[320 * 200];
uint8_t previousFrame[320 * 200];
COLORREF transparencyRef;
extern HWND HWnd;
typedef int ESoundDriver;
extern HINSTANCE hInst;
float multiplier;

extern ESoundDriver soundDriver;

void setupOPL2(int port) {}

void stopSounds() {}

#ifdef MSVC
//char* files[] = {"menu_move.raw", "menu_select.raw", "gotclue.raw", "detected2.raw", "bong.raw", "fire.raw", "enemyfire.raw", "detected2.raw" };
int resId[] = {IDR_WAVE10, IDR_WAVE1, IDR_WAVE8, IDR_WAVE4, IDR_WAVE11, IDR_WAVE7, IDR_WAVE6, IDR_WAVE4 };

void playSound( const int action ){	
	PlaySound(MAKEINTRESOURCE(resId[action]), hInst, SND_RESOURCE | SND_ASYNC);
}
#else
void playSound( const int action ){
}
#endif
void soundTick() {}

void muteSound() {}


long frame = 0;

void graphicsShutdown() {
    texturesUsed = 0;

}

void putStr(int x, int y, const char *str, int fg, int bg) {

}

void drawTitleBox() {

}

void querySoundDriver() {

}


static HBITMAP Create8bppBitmap(HDC hdc, int width, int height, LPVOID pBits)
{
    int I, r, g, b;
    uint32_t pixel;
	void *Pixels;
	HBITMAP hbmp;
    BITMAPINFO *bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
    PBITMAPINFOHEADER bih;


    bih = &bmi->bmiHeader;

    bih->biSize = sizeof (BITMAPINFOHEADER);
    bih->biWidth         = width;
    bih->biHeight        = -height;
    bih->biPlanes        = 1;
    bih->biBitCount      = 8;
    bih->biCompression   = BI_RGB;
    bih->biSizeImage     = 0;
    bih->biXPelsPerMeter = 14173;
    bih->biYPelsPerMeter = 14173;
    bih->biClrUsed       = 0;
    bih->biClrImportant  = 0;



    memcpy( &bmi->bmiColors[0], &paletteRef[0], 256 * sizeof(RGBQUAD));


    Pixels = NULL;
    hbmp = CreateDIBSection(hdc, bmi, DIB_RGB_COLORS, &Pixels, NULL, 0);

    if(pBits != NULL)
    {
        BYTE* pbBits = (BYTE*)pBits;
        BYTE *Pix = (BYTE *)Pixels;
        memcpy(Pix, pbBits, width * height);
    }

    free(bmi);

    return hbmp;
}


void setMultiplier(RECT* size) {
	int width = size->right - size->left;
	int height = size->bottom - size->top;
	
    if (((320.0f / 240.0f ) * height) <  width ) {
        multiplier = (((float) height) / 240.0f);
    } else {
        multiplier = (((float) width) / 320.0f);
    }
}

static HBITMAP CreateBitmapFromPixels( HDC hDC, UINT uWidth, UINT uHeight, UINT uBitsPerPixel, LPVOID pBits)
{
	HBITMAP hBitmap;
	LONG lBmpSize;
	BITMAPINFO bmpInfo;
	UINT * pPixels;

    if(uBitsPerPixel < 8)
        return NULL;

    if(uBitsPerPixel == 8)
        return Create8bppBitmap(hDC, uWidth, uHeight, pBits);

    hBitmap = 0;
    if ( !uWidth || !uHeight || !uBitsPerPixel )
        return hBitmap;
    lBmpSize = uWidth * uHeight * (uBitsPerPixel/8) ;
	memset(&bmpInfo, 0, sizeof(bmpInfo));
    bmpInfo.bmiHeader.biBitCount = uBitsPerPixel;
    bmpInfo.bmiHeader.biHeight = uHeight;
    bmpInfo.bmiHeader.biWidth = uWidth;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    pPixels = 0;
    hBitmap = CreateDIBSection( hDC, (BITMAPINFO *)&
                                     bmpInfo, DIB_RGB_COLORS, (void **)&
            pPixels , NULL, 0);

    if ( !hBitmap )
        return hBitmap;

    memcpy(pPixels, pBits, lBmpSize );

    return hBitmap;
}

uint8_t getPaletteEntry(const uint32_t origin) {
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

    int r, g, b, index = 0;
    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                palette[paletteEntry] = pixel;
                paletteRef[paletteEntry].rgbReserved = 0;
                paletteRef[paletteEntry].rgbRed = (BYTE)((pixel & 0x000000FF) - 0x38);
                paletteRef[paletteEntry].rgbGreen = (BYTE)(((pixel & 0x0000FF00) >> 8) - 0x18);
                paletteRef[paletteEntry].rgbBlue = (BYTE)(((pixel & 0x00FF0000) >> 16) - 0x10);
                index++;
            }
        }
    }
	enableSmoothMovement = TRUE;
    defaultFont = loadBitmap("font.img");
}





LRESULT CALLBACK
WindProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    HDC hDC;

    InvalidateRect(hWnd, NULL, 0);

    switch (Msg) {

        case WM_CHAR:
            switch (wParam) {

                case 's':
                mBufferedCommand = kCommandStrafeLeft;
                visibilityCached = FALSE;
                break;

                case 'd':
                mBufferedCommand = kCommandStrafeRight;
                visibilityCached = FALSE;
                break;

                case 'z':
                mBufferedCommand = kCommandFire1;
                visibilityCached = FALSE;
                break;

                case 'x':
                mBufferedCommand = kCommandFire2;
                visibilityCached = FALSE;
                break;

                case 'c':
                mBufferedCommand = kCommandFire3;
                visibilityCached = FALSE;
                break;

                case 'v':
                mBufferedCommand = kCommandFire4;
                visibilityCached = FALSE;
                break;

                case 'q':
                mBufferedCommand = kCommandBack;
                break;
            }
            needsRedraw = TRUE;
        break;

        case
        WM_KEYDOWN:

        switch (wParam) {

            case
            VK_ESCAPE:
            mBufferedCommand = kCommandBack;
            break;
            case
            VK_SPACE:
            mBufferedCommand = kCommandFire2;
            visibilityCached = FALSE;
            break;


            case
            VK_LEFT:
            mBufferedCommand = kCommandLeft;
            visibilityCached = FALSE;
			turnStep = 0;
			turnTarget = 200;
            break;

            case
            VK_RIGHT:
            mBufferedCommand = kCommandRight;
            visibilityCached = FALSE;
			turnStep = 200;
			turnTarget = 0;
            break;

            case
            VK_UP:
            mBufferedCommand = kCommandUp;
            visibilityCached = FALSE;
            break;

            case
            VK_RETURN:
            mBufferedCommand = kCommandFire1;
            break;


            case
            VK_DOWN:
            mBufferedCommand = kCommandDown;
            visibilityCached = FALSE;

            break;
        }
        needsRedraw = TRUE;

        break;

        case WM_DESTROY:
            PostQuitMessage(WM_QUIT);
            mBufferedCommand = kCommandQuit;
            break;

        case WM_PAINT:
            needsRedraw = TRUE;
        break;
        default:
          return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    if (needsRedraw) {
        PAINTSTRUCT     ps;
        HDC             hdc;
        BITMAP          bitmap;
        HDC             hdcMem;
        HGDIOBJ         oldBitmap;
        HBITMAP hBitmap;
		RECT rect;
		int y, x, bufferW, bufferH, trueW, trueH;
		float yMultiplier;

		GetClientRect(hWnd, &rect);
		setMultiplier(&rect);
		
		yMultiplier = ( (240.0f * multiplier) / 200.0f );

        hdc = BeginPaint(hWnd, &ps);

        hdcMem = CreateCompatibleDC(hdc);

////////////////
	

	if ( !enableSmoothMovement || turnTarget == turnStep ) {
		memcpy( slideBitmap, framebuffer, 320 * 200);
		memcpy( previousFrame, framebuffer, 320 * 200);
	} else if ( turnStep < turnTarget ) {

		for ( y = dirtyLineY0; y < dirtyLineY1; ++y ) {
			for ( x = 0; x < 320; ++x ) {
				uint8_t index;

				if (x < XRES  ) {

					if ( x  >= turnStep ) {
						index = previousFrame[ (320 * y) - turnStep + x ];
					} else {
						index = framebuffer[ (320 * y) + x - (320 - XRES) - turnStep];
					}

				} else {
					index = framebuffer[ (320 * y) + x];
				}

				slideBitmap[(320*y) + x] = index;
			}
		}

		turnStep+= 20;
	} else {


		for ( y = dirtyLineY0; y < dirtyLineY1; ++y ) {
			for ( x = 0; x < 320; ++x ) {
				uint8_t index;

				if (x < XRES ) {

					if ( x  >= turnStep ) {
						index = framebuffer[ (320 * y) - turnStep + x ];
					} else {
						index = previousFrame[ (320 * y) + x - (320 - XRES) - turnStep];
					}

				} else {
					index = framebuffer[ (320 * y) + x];
				}

				slideBitmap[(320*y) + x] = index;
			}
		}

		turnStep-= 20;
	}








//////////////

        hBitmap = CreateBitmapFromPixels(hdcMem, 320, 200, 8, &slideBitmap[0]);

        oldBitmap = SelectObject(hdcMem, hBitmap);

        GetObject(hBitmap, sizeof(bitmap), &bitmap);


		bufferW = (320 * multiplier);
		bufferH = (200 * yMultiplier);
		trueW = (rect.right - rect.left);
		trueH = (rect.bottom - rect.top);


		StretchBlt(hdc, ( trueW - bufferW) / 2, (trueH - bufferH) / 2, bufferW, bufferH, hdcMem, 0, 0, 320, 200, SRCCOPY); 

        SelectObject(hdcMem, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
    }

    return 0;
}

void handleSystemEvents() {
    MSG msg;
    BOOL bRet;


    if (bRet = GetMessage(&msg, NULL, 0, 0)) {
        if (bRet != -1) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void flipRenderer() {
    InvalidateRect(HWnd, NULL, 0);
}

void clear() {}




