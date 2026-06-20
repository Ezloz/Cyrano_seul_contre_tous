#pragma once
#include "Camera.h"
#include "Character.h"
#include "GameTypes.h"
#include "MapLayer.h"
#include "TurnQueue.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <memory>
#include <set>
#include <string>
#include <tmxlite/ObjectGroup.hpp>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Camera;

class Map : public sf::Drawable {
private:
  tmx::Map tmxMap;
  std::unique_ptr<Camera> activeCamera;
  std::vector<std::unique_ptr<MapLayer>> layers;
  std::unique_ptr<MapLayer> cursorLayer;
  tmx::Vector2u tileSize;
  Coord viewSize = {15, 10};
  Coord edgeOffset = {3, 2};

  std::mutex charactersMutex;
  std::vector<std::unique_ptr<Character>> characters;

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    for (const auto &layer : layers) {
      target.draw(*layer, states);
    }
    if (cursorLayer && activeCamera->isCursorVisible()) {
      target.draw(*cursorLayer, states);
    }
  }

public:
  explicit Map(const std::string &name, int nbLayer = 1);
  ~Map();

  tmx::Map *GetTMXMap() { return &tmxMap; };

  tmx::Vector2u GetTileSize() const { return tileSize; };

  Coord GetViewSize() const { return viewSize; };

  void move(std::set<Input> inputs, std::set<Input> inputsRelease,
            sf::Time deltaTime);

  void startCinematic(Coord from, Coord to, sf::Time duration);
  bool isCinematicActive() const;

  void update(sf::Time elapsed);

  static Map loadMap(std::string save) {
    printf("%s\n", save.c_str());
    json saveJson = openJson(save.c_str());
    if (saveJson.contains("map")) {
      std::string mapId = saveJson["map"].get<std::string>();
      printf("%s\n", mapId.c_str());
      json data = openJson(DATASET);
      std::string tmxMap = data["maps"][mapId].get<std::string>();
      printf("%s\n", tmxMap.c_str());
      return Map(tmxMap, 1);
    }
    // for (auto it = data.at("map").begin();
    //      it != data.at(std::string("entryPoint") + slot).end(); ++it) {
    //   // textureDataset[it.key()] = it.value();
    // }
  };
};
