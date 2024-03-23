#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <sstream>
#include "editor-core.h"

#include <iostream>

int main(int argc, char** argv) {
  bool quit = false;
  editor::Map map;

  while (!quit) {
    std::string input;

    std::cout << to_string(map) << std::endl;
    
    std::cout << ">";
    std::cin >> input;

    if ("q" == input || "quit" == input) {
      quit = true;
    } else if ("set-connection" == input) {
      int slot;
      uint8_t connection;
      std::cin >> slot;
      std::cin >> connection;
      
      map.setConnetion(slot, connection);
      std::cout << "connecting to " << connection << " over " << slot << std::endl;
    } else if ("create-area" == input) {
      int x0, dx, y0, dy, z0, dz;

      std::cin >> x0;
      std::cin >> y0;
      std::cin >> z0;
      std::cin >> dx;
      std::cin >> dy;
      std::cin >> dz;

      map.createArea(x0, y0, z0, dx, dy, dz);
    }
    
  }

  return 0;
}
