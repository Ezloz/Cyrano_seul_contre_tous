#include "Camera.h"
#include "Map.h"

Map::Map(const std::string &name, int nbLayer) {
  tmxMap.load(name);
  activeCamera = std::make_unique<Camera>(0, 0, 5, 5, tmxMap);
  for (int i = 0; i < nbLayer; i++) {
    layers.push_back(std::make_unique<MapLayer>(tmxMap, i));
  }
}

Map::~Map() = default;

void Map::move(std::set<Input> inputs, std::set<Input> inputsRelease) {
  sf::Vector2f offset = activeCamera->processNewOffset(inputs, inputsRelease);
  for (auto &layer : layers) {
    layer->setOffset(offset);
  }
}
