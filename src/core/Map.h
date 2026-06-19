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
  std::vector<std::unique_ptr<Character>> ennemies;
  std::vector<std::unique_ptr<Character>> allies;

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    for (const auto &layer : layers) {
      target.draw(*layer, states);
    }
    if (cursorLayer) {
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

  void update(sf::Time elapsed);
};
