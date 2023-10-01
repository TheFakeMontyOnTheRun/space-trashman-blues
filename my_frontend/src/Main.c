#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"
#include "SoundSystem.h"

typedef void ( *InitStateCallback )(int32_t tag);

typedef void ( *InitialPaintCallback )(void);

typedef void ( *RepaintCallback )(void);

typedef enum EGameMenuState ( *TickCallback )(enum ECommand, long data);

typedef void ( *UnloadStateCallback )(int32_t newState);

const char *mainText;
int isRunning = 1;

long timeUntilNextState;
enum EPresentationState currentPresentationState;
enum EGameMenuState currentGameMenuState;
int8_t cameraRotation = 0;
int cursorPosition;
enum EGameMenuState nextNavigationSelection;
enum EGameMenuState menuStateToReturn;


void stopSounds(void) {}

void handleSystemEvents(void) {}

void soundTick(void) {}

void playSound( int ununused ) {}

enum ECommand getInput() {
    return kCommandNone;
}

int main(int argc, char **argv) {
  initState(1);

  titleScreen();
  initMap();

  do {
    tickRenderer();
    tickGame(kCommandNone, 1);
  } while (isRunning);

  shutdownGraphics();

  return 0;
}
