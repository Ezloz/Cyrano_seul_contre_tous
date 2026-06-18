#include "Map.h"

Map::Map(const std::string &name, int nbLayer) {
  tmxMap.load(name);
  tileSize = tmxMap.getTileSize();
  activeCamera = std::make_unique<Camera>(0, 0, edgeOffsetX, edgeOffsetY,
                                          viewSizeX, viewSizeY, tmxMap);
  for (int i = 0; i < nbLayer; i++) {
    layers.push_back(std::make_unique<MapLayer>(tmxMap, i));
  }
  tmxMap.load("resources/cursor.tmx");
  cursorLayer = std::make_unique<MapLayer>(tmxMap, 0);
}

Map::~Map() = default;

void Map::move(std::set<Input> inputs, std::set<Input> inputsRelease,
               sf::Time deltaTime) {
  activeCamera->processNewOffset(inputs, inputsRelease, deltaTime);
  cursorLayer->setOffset(activeCamera->getCursorOffset());
  for (auto &layer : layers) {
    layer->setOffset(activeCamera->getMapOffset());
  }
}

void Map::update(sf::Time elapsed) {
  for (auto &layer : layers) {
    layer->update(elapsed);
  }
  if (cursorLayer) {
    cursorLayer->update(elapsed);
  }

  // Character Map::getCharacter(std::string id) {
  //   for (const auto &character : characters.getObjects()) {
  //     if (character->id == id) {
  //       return character;
  //     }
  //   }
};
