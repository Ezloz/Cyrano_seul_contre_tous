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
  std::int32_t cursorX;
  std::int32_t cursorY;
  std::int32_t previousCursorX;
  std::int32_t previousCursorY;
  sf::Vector2f cursorOffset;

  std::int32_t mapCornerX;
  std::int32_t mapCornerY;
  std::int32_t previousMapCornerX;
  std::int32_t previousMapCornerY;
  sf::Vector2f mapOffset;
  std::int32_t edgeOffsetX;
  std::int32_t edgeOffsetY;

  std::int32_t mapSizeX;
  std::int32_t mapSizeY;
  std::int32_t viewSizeX;
  std::int32_t viewSizeY;
  tmx::Vector2u tileSize;

  sf::Time repeatRate;
  sf::Time repeatDelay;
  sf::Time lastMove;

public:
  bool isPressed[static_cast<int>(Input::NB_INPUTS)];
  bool delayedMove;
  bool freshPress;
  std::int32_t dx;
  std::int32_t dy;

  Camera(std::int32_t initialCursorX, std::int32_t initialCursorY,
         std::int32_t edgeOffsetX, std::int32_t edgeOffsetY,
         std::int32_t viewSizeX, std::int32_t viewSizeY, tmx::Map &map) {
    cursorX = initialCursorX;
    cursorY = initialCursorY;
    previousCursorX = initialCursorX;
    previousCursorY = initialCursorY;
    mapCornerX = 0;
    mapCornerY = 0;
    previousMapCornerX = 0;
    previousMapCornerY = 0;
    this->edgeOffsetX = edgeOffsetX;
    this->edgeOffsetY = edgeOffsetY;
    this->viewSizeX = viewSizeX;
    this->viewSizeY = viewSizeY;
    tileSize = map.getTileSize();
    mapSizeX = map.getTileCount().x;
    mapSizeY = map.getTileCount().y;
    repeatRate = sf::milliseconds(85);
    repeatDelay = sf::milliseconds(250);
    std::fill(isPressed, isPressed + static_cast<int>(Input::NB_INPUTS), false);
    delayedMove = false;
    freshPress = false;
    dx = 0;
    dy = 0;
  }
  sf::Vector2f getCursorOffset() { return cursorOffset; }
  sf::Vector2f getMapOffset() { return mapOffset; }
  void processNewOffset(std::set<Input> inputs, std::set<Input> releaseInputs,
                        sf::Time deltaTime);
};
