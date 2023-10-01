#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"
#include "Engine.h"
#include "SoundSystem.h"

const char *mainText;
int isRunning = 1;

long timeUntilNextState;
enum EPresentationState currentPresentationState;
enum EGameMenuState currentGameMenuState;
int8_t cameraRotation = 0;
int cursorPosition;
enum EGameMenuState nextNavigationSelection;
enum EGameMenuState menuStateToReturn;

int main(int argc, char **argv) {

    enterState(kPlayGame);
   // titleScreen();

  do {
      repaintCallback();
      tickCallback(kCommandNone, 1);
  } while (isRunning);

    unloadStateCallback(1);

  return 0;
}
