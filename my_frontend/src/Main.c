void initState(void);
void tickRenderer(void);
void shutdownGraphics(void);

extern int running;

int main(int argc, char **argv) {
  initState();

  do {
    tickRenderer();
  } while (running);

  shutdownGraphics();

  return 0;
}
