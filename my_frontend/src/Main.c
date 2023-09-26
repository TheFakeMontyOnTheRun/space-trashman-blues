#include <stdint.h>
#include "Menu.h"
#include "Engine3D.h"

void initState(void);
void tickRenderer(void);
void shutdownGraphics(void);

extern int running;

int main(int argc, char **argv) {
  initState();

  titleScreen();
  initMap();

  do {
    tickRenderer();
  } while (running);

  shutdownGraphics();

  return 0;
}
