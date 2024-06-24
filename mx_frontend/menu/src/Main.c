#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef AMIGA
#include "AmigaInt.h"

#define VERSTAG "\0$VER: Sub Mare Imperium - Derelict 1.0 (10.10.2023)"
#else

#ifdef WIN32
#include <windows.h>
#include "Win32Int.h"

#ifdef MSVC
#include "resource.h"
#endif

#else

#include <stdint.h>

#endif


#endif

#ifdef WIN32
const long UCLOCKS_PER_SEC = 1000;
long uclock(void) {
    SYSTEMTIME systime;
    GetSystemTime(&systime);


    return 1000 * ((systime.wSecond * 1000) + (systime.wMilliseconds));
}
#else
#ifndef __DJGPP__
const long UCLOCKS_PER_SEC = 1000;

long timeEllapsed = 0;

long uclock(void) {
    timeEllapsed += (1000 / 60);
    return timeEllapsed;
}

#endif
#endif

#include "Common.h"
#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "Renderer.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"
#include "Derelict.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

char *textBuffer;
extern char *messageLogBuffer;
extern enum EVisibility *visMap;
extern struct Vec2i *distances;
extern uint8_t *collisionMap;
extern struct Texture *textures;

struct Texture internalTexturesMem[TOTAL_TEXTURES];

void initHW(int argc, char** argv) {
    textBuffer = (char *) allocMem(TEXT_BUFFER_SIZE, GENERAL_MEMORY, 1);
    messageLogBuffer = (char *) allocMem(256, GENERAL_MEMORY, 1);
    collisionMap = (uint8_t *) allocMem(256, GENERAL_MEMORY, 1);
    visMap = (enum EVisibility *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(enum EVisibility), GENERAL_MEMORY, 1);
    distances = (struct Vec2i *) allocMem(2 * MAP_SIZE * MAP_SIZE * sizeof(struct Vec2i), GENERAL_MEMORY, 1);
    textures = &internalTexturesMem[0];
    itemsInMap = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);
    map = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);

#ifndef CD32
    initFileReader("base.pfs");
#else
    initFileReader("base.pfs");
#endif
    initZMap();
    graphicsInit();
}

void shutdownHW(void) {
    graphicsShutdown();

    disposeMem(textBuffer);
    disposeMem(messageLogBuffer);
    disposeMem(collisionMap);
    disposeMem(visMap);
    disposeMem(distances);
    disposeMem(itemsInMap);
    disposeMem(map);
}

long start_clock, end_clock, prev;

#ifdef __EMSCRIPTEN__
void mainLoop (void) {
  menuTick ( 50 );
  flipRenderer();
}
#endif

#ifndef ANDROID


#ifdef WIN32

HWND HWnd;
HINSTANCE hInst;

LRESULT CALLBACK WindProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{

        int argc = 0;
		char **argv = NULL;

        WNDCLASSEX WndCls;
        RECT window_rect = {0, 0, 640, 480};
        static char szAppName[] = "Sub Mare Imperium Derelict 95";

        hInst = hInstance;
        WndCls.cbSize = sizeof(WndCls);
        WndCls.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        WndCls.lpfnWndProc = WindProcedure;
        WndCls.cbClsExtra = 0;
        WndCls.cbWndExtra = 0;
        WndCls.hInstance = hInst;
        WndCls.hCursor = LoadCursor(NULL, IDC_ARROW);
        WndCls.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
        WndCls.lpszMenuName = NULL;
        WndCls.lpszClassName = szAppName;
#ifdef MSVC
        WndCls.hIcon = LoadIcon(NULL, IDI_SMALL);
        WndCls.hIconSm = LoadIcon(hInstance, IDI_THEMISTRALREPORT95);
#endif
        RegisterClassEx(&WndCls);


        HWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                       szAppName,
                       "Sub Mare Imperium Derelict 95",
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       640,
                       480,
                       NULL,
                       NULL,
                       hInstance,
                       NULL);


        AdjustWindowRectEx(&window_rect,
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            FALSE,
                            WS_EX_OVERLAPPEDWINDOW
                            );

        MoveWindow(HWnd,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    window_rect.right - window_rect.left,
                    window_rect.bottom - window_rect.top,
                    TRUE
                    );


#else

int main(int argc, char **argv) {
#endif
    initHW(argc, argv);
    initStation();
    enterState(kMainMenu);

    end_clock = uclock();
    prev = 0;

    start_clock = uclock();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (isRunning) {
        long now, delta_time;

#ifdef AMIGA
#ifdef AGA8BPP
        delta_time = 50;
#else
        delta_time = 1000;
#endif

#else
        now = (end_clock - start_clock) / (UCLOCKS_PER_SEC / 1000);
        delta_time = now - prev;
        prev = now;

        /* protect against machines too fast for their own good. */
        if (delta_time < 50) {
            delta_time = 50;
        }
#endif
        isRunning = isRunning && menuTick(delta_time);
        flipRenderer();
    }
#endif
    unloadStateCallback(-1);
    shutdownHW();

    return 0;
}

#endif
