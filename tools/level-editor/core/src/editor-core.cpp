#include <string>
#include <cstdint>
#include <vector>

#include "editor-core.h"

void Map::createArea(uint8_t x0, uint8_t y0, uint8_t dx, uint8_t dy, uint8_t ceilingHeight) {
}

uint8_t Map::getConnection(int slot) {
  return connections[slot];
}

void Map::setConnetion(int slot, uint8_t connection) {
  connections[slot] = connection;
}

void Map::setName(const std::string& newName) {
  name = newName;
}

std::string Map::getName() {
  return name;
}


void saveMap(const Map& map) {
}
