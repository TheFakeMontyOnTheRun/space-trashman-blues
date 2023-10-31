#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"
#include "Engine.h"
#include "SoundSystem.h"

int8_t cameraRotation = 0;

int main(int argc, char **argv) {
    initHW();
    enterState(kMainMenu);

  do {
      menuTick(10);
      graphicsFlush();
  } while (isRunning);

    unloadStateCallback(1);

  return 0;
}
