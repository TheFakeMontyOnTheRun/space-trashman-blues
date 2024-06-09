#ifdef WIN32
#include "Win32Int.h"
#else
#ifndef SMD
#include <stdint.h>
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif
#endif

#ifdef ATARIST
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mint/osbind.h>
#include <mint/sysbind.h>
#endif

#include "Common.h"
#include "Enums.h"
#include "Renderer.h"
#include "Engine.h"
#include "Core.h"
#include "UI.h"

int8_t cameraRotation = 0;

#ifdef ATARIST
int doMain(void);

int main(int argc, char **argv) {
    Supexec(&doMain);
    return 0;
}

int doMain(void) {
    int argc = 0;
    char **argv = NULL;
#else
int main(int argc, char **argv) {
#endif
    initFileReader("base.pfs");
    initHW(argc, argv);


    enterState(kMainMenu);

#ifdef EMIT_QUIT_OPTION
    while (isRunning) {
#else
    while (1) {
#endif
      startFrame(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER);
      menuTick(10);
      endFrame();
  }

#ifdef EMIT_QUIT_OPTION
    unloadStateCallback(1);
    return 0;
#endif
}
