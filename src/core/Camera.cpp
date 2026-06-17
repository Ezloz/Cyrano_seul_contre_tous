#include "Camera.h"
#include <algorithm>
#include <set>

void Camera::processNewOffset(std::set<Input> inputs,
                              std::set<Input> releaseInputs,
                              sf::Time deltaTime) {
  const int UP = static_cast<int>(Input::UP);
  const int DOWN = static_cast<int>(Input::DOWN);
  const int LEFT = static_cast<int>(Input::LEFT);
  const int RIGHT = static_cast<int>(Input::RIGHT);

  for (const auto &input : inputs) {
    if (input == Input::UP || input == Input::DOWN || input == Input::LEFT ||
        input == Input::RIGHT) {
      int idx = static_cast<int>(input);
      if (!isPressed[idx]) {
        isPressed[idx] = true;
        freshPress = true;
      }
    }
  }

  this->lastMove += deltaTime;
  bool doStep = false;

  if (dx == 0 && dy == 0 && freshPress) {
    dx = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
    dy = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);
    doStep = true;
    delayedMove = true;
    this->lastMove = sf::Time::Zero;
  }

  if (dx != 0 || dy != 0) {
    if (delayedMove && this->lastMove >= repeatDelay) {
      doStep = true;
      delayedMove = false;
      this->lastMove %= repeatDelay;
    } else if (!delayedMove && this->lastMove >= repeatRate) {
      doStep = true;
      this->lastMove %= repeatRate;
    }
  }

  if (doStep) {
    previousCursorX = cursorX;
    previousCursorY = cursorY;
    previousMapCornerX = mapCornerX;
    previousMapCornerY = mapCornerY;
    dx = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
    dy = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);

    cursorX = std::clamp(previousCursorX + dx, 0, mapSizeX - 1);
    cursorY = std::clamp(previousCursorY + dy, 0, mapSizeY - 1);

    const std::int32_t maxCornerX = std::max(0, mapSizeX - viewSizeX);
    const std::int32_t maxCornerY = std::max(0, mapSizeY - viewSizeY);

    if (cursorX - mapCornerX < edgeOffsetX) {
      mapCornerX = cursorX - edgeOffsetX;
    } else if (cursorX - mapCornerX > viewSizeX - 1 - edgeOffsetX) {
      mapCornerX = cursorX - (viewSizeX - 1 - edgeOffsetX);
    }
    mapCornerX = std::clamp(mapCornerX, 0, maxCornerX);

    if (cursorY - mapCornerY < edgeOffsetY) {
      mapCornerY = cursorY - edgeOffsetY;
    } else if (cursorY - mapCornerY > viewSizeY - 1 - edgeOffsetY) {
      mapCornerY = cursorY - (viewSizeY - 1 - edgeOffsetY);
    }
    mapCornerY = std::clamp(mapCornerY, 0, maxCornerY);
  }

  for (const auto &release : releaseInputs) {
    isPressed[static_cast<int>(release)] = false;
  }

  // if (this->lastMove >= repeatRate) {
  //   previousCursorX = cursorX;
  //   previousCursorY = cursorY;
  //   previousMapCornerX = mapCornerX;
  //   previousMapCornerY = mapCornerY;
  //   dx = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
  //   dy = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);
  // }

  float ratio =
      std::min(1.f, this->lastMove.asSeconds() / repeatRate.asSeconds());

  float cursorTileX = previousCursorX + (cursorX - previousCursorX) * ratio;
  float cursorTileY = previousCursorY + (cursorY - previousCursorY) * ratio;
  float cornerTileX =
      previousMapCornerX + (mapCornerX - previousMapCornerX) * ratio;
  float cornerTileY =
      previousMapCornerY + (mapCornerY - previousMapCornerY) * ratio;

  mapOffset =
      sf::Vector2f(-cornerTileX * tileSize.x, -cornerTileY * tileSize.y);

  cursorOffset = sf::Vector2f((cursorTileX - cornerTileX) * tileSize.x,
                              (cursorTileY - cornerTileY) * tileSize.y) -
                 sf::Vector2f(3.0f, 3.0f);
}
