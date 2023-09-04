void initState(void);
void tickRenderer(void);
void shutdownGraphics(void);

int main(int argc, char **argv) {
  initState();

  do {
    tickRenderer();
  } while (running);

  shutdownGraphics();

  return 0;
}
