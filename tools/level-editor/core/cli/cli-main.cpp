#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include "editor-core.h"

#include <iostream>

int main(int argc, char** argv) {
  bool quit = false;
  Map map;
  std::cout << "The most amazing level editor ever!" << std::endl;

  while (!quit) {
    std::string input;
    std::cout << ">";
    std::cin >> input;

    if ("q" == input || "quit" == input) {
      quit = true;
    }
    
    if ("set-connection" == input) {
      int slot;
      uint8_t connection;
      std::cin >> slot;
      std::cin >> connection;
      
      map.setConnetion(slot, connection);
      std::cout << "connecting to " << connection << " over " << slot << std::endl;
    }
    
  }

  return 0;
}
