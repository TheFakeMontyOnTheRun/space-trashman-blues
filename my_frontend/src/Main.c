#include <stdint.h>
#include "UI.h"
#include "Renderer.h"

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
