#pragma once
#include "GameTypes.h"
#include <algorithm>
#include <tmxlite/Map.hpp>

#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>

struct Camera {
private:
  Coord cursor;
  Coord previousCursor;
  sf::Vector2f cursorOffset;

  Coord mapCorner;
  Coord previousMapCorner;
  sf::Vector2f mapOffset;
  Coord edgeOffset;

  Coord mapSize;
  Coord viewSize;
  tmx::Vector2u tileSize;

  sf::Time repeatRate;
  sf::Time repeatDelay;
  sf::Time lastMove;

public:
  bool isPressed[static_cast<int>(Input::NB_INPUTS)];
  bool delayedMove;
  bool freshPress;
  Coord delta;

  Camera(Coord initialCursor, Coord edgeOffset, Coord viewSize, tmx::Map &map) {
    cursor = initialCursor;
    previousCursor = initialCursor;
    mapCorner = {0, 0};
    previousMapCorner = {0, 0};
    this->edgeOffset = edgeOffset;
    this->viewSize = viewSize;
    tileSize = map.getTileSize();
    mapSize = {static_cast<int>(map.getTileCount().x),
               static_cast<int>(map.getTileCount().y)};
    repeatRate = sf::milliseconds(85);
    repeatDelay = sf::milliseconds(250);
    std::fill(isPressed, isPressed + static_cast<int>(Input::NB_INPUTS), false);
    delayedMove = false;
    freshPress = false;
    delta = {0, 0};
  }
  sf::Vector2f getCursorOffset() { return cursorOffset; }
  sf::Vector2f getMapOffset() { return mapOffset; }
  void processNewOffset(std::set<Input> inputs, std::set<Input> releaseInputs,
                        sf::Time deltaTime);
};
