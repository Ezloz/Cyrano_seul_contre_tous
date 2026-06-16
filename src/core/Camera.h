#pragma once
#include "GameApp.h"
#include "MapLayer.h"
#include <algorithm>

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
  std::int32_t edgeOffsetX;
  std::int32_t edgeOffsetY;
  std::int32_t cameraSizeX;
  std::int32_t cameraSizeY;
  std::int32_t mapSizeX;
  std::int32_t mapSizeY;
  tmx::Vector2u tileSize;

  sf::Time repeatRate;
  sf::Time repeatDelay;
  sf::Time lastMove;

public:
  bool isPressed[static_cast<int>(Input::NB_INPUTS)];
  bool delayedMove;

  Camera(std::int32_t initialCursorX, std::int32_t initialCursorY,
         std::int32_t edgeOffsetX, std::int32_t edgeOffsetY, tmx::Map &map) {
    cursorX = initialCursorX;
    cursorY = initialCursorY;
    previousCursorX = initialCursorX;
    previousCursorY = initialCursorY;
    this->edgeOffsetX = edgeOffsetX;
    this->edgeOffsetY = edgeOffsetY;
    cameraSizeX = 10;
    cameraSizeY = 10;
    tileSize = map.getTileSize();
    mapSizeX = map.getTileCount().x;
    mapSizeY = map.getTileCount().y;
    repeatRate = sf::milliseconds(85);
    repeatDelay = sf::milliseconds(250);
    std::fill(isPressed, isPressed + static_cast<int>(Input::NB_INPUTS), false);
    delayedMove = true;
  }
  sf::Vector2f processNewOffset(std::set<Input> inputs,
                                std::set<Input> releaseInputs);
};
