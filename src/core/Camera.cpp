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

  for (const auto &release : releaseInputs) {
    isPressed[static_cast<int>(release)] = false;
    freshPress = true;
    delta.x = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
    delta.y = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);
  }

  this->lastMove += deltaTime;
  bool doStep = false;

  if (freshPress) {
    this->lastMove = sf::Time::Zero;
    doStep = true;
    delta.x = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0) - delta.x;
    delta.y = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0) - delta.y;
    printf("%d, %d\n", delta.x, delta.y);
    delayedMove = true;
    freshPress = false;
  } else {
    if (delayedMove && this->lastMove >= repeatDelay) {
      delta.x = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
      delta.y = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);
      doStep = true;
      delayedMove = false;
      this->lastMove %= repeatDelay;
    } else if (!delayedMove && this->lastMove >= repeatRate) {
      delta.x = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
      delta.y = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);
      doStep = true;
      this->lastMove %= repeatRate;
    }
  }

  if (doStep) {
    previousCursor = cursor;
    previousMapCorner = mapCorner;

    cursor.x = std::clamp(previousCursor.x + delta.x, 0, mapSize.x - 1);
    cursor.y = std::clamp(previousCursor.y + delta.y, 0, mapSize.y - 1);

    const std::int32_t maxCornerX = std::max(0, mapSize.x - viewSize.x);
    const std::int32_t maxCornerY = std::max(0, mapSize.y - viewSize.y);

    if (cursor.x - mapCorner.x < edgeOffset.x) {
      mapCorner.x = cursor.x - edgeOffset.x;
    } else if (cursor.x - mapCorner.x > viewSize.x - 1 - edgeOffset.x) {
      mapCorner.x = cursor.x - (viewSize.x - 1 - edgeOffset.x);
    }
    mapCorner.x = std::clamp(mapCorner.x, 0, maxCornerX);

    if (cursor.y - mapCorner.y < edgeOffset.y) {
      mapCorner.y = cursor.y - edgeOffset.y;
    } else if (cursor.y - mapCorner.y > viewSize.y - 1 - edgeOffset.y) {
      mapCorner.y = cursor.y - (viewSize.y - 1 - edgeOffset.y);
    }
    mapCorner.y = std::clamp(mapCorner.y, 0, maxCornerY);
  }

  // if (this->lastMove >= repeatRate) {
  //   previousCursor = cursor;
  //   previousMapCorner = mapCorner;
  //   delta.x = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
  //   delta.y = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);
  // }

  float ratio =
      std::min(1.f, this->lastMove.asSeconds() / repeatRate.asSeconds());

  float cursorTileX = previousCursor.x + (cursor.x - previousCursor.x) * ratio;
  float cursorTileY = previousCursor.y + (cursor.y - previousCursor.y) * ratio;
  float cornerTileX =
      previousMapCorner.x + (mapCorner.x - previousMapCorner.x) * ratio;
  float cornerTileY =
      previousMapCorner.y + (mapCorner.y - previousMapCorner.y) * ratio;

  mapOffset =
      sf::Vector2f(-cornerTileX * tileSize.x, -cornerTileY * tileSize.y);

  cursorOffset = sf::Vector2f((cursorTileX - cornerTileX) * tileSize.x,
                              (cursorTileY - cornerTileY) * tileSize.y) -
                 sf::Vector2f(3.0f, 3.0f);
}
