#include <stdint.h>
#include "Enums.h"
#include "Renderer.h"
#include "Engine.h"

int8_t cameraRotation = 0;

int main(int argc, char **argv) {
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
