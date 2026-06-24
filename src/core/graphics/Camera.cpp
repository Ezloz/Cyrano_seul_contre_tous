#include "graphics/Camera.h"
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

    if (cursor.x - mapCorner.x < edgeOffset.x) {
      mapCorner.x = cursor.x - edgeOffset.x;
    } else if (cursor.x - mapCorner.x > viewSize.x - 1 - edgeOffset.x) {
      mapCorner.x = cursor.x - (viewSize.x - 1 - edgeOffset.x);
    }
    mapCorner.x = std::clamp(mapCorner.x, 0, maxCornerMap.x);

    if (cursor.y - mapCorner.y < edgeOffset.y) {
      mapCorner.y = cursor.y - edgeOffset.y;
    } else if (cursor.y - mapCorner.y > viewSize.y - 1 - edgeOffset.y) {
      mapCorner.y = cursor.y - (viewSize.y - 1 - edgeOffset.y);
    }
    mapCorner.y = std::clamp(mapCorner.y, 0, maxCornerMap.y);
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

void Camera::setCursor(Coord c) {
  cursor.x = std::clamp(c.x, 0, mapSize.x - 1);
  cursor.y = std::clamp(c.y, 0, mapSize.y - 1);
  previousCursor = cursor;

  // Recentre la caméra pour garder le curseur dans la vue (même logique que
  // processNewOffset).
  if (cursor.x - mapCorner.x < edgeOffset.x) {
    mapCorner.x = cursor.x - edgeOffset.x;
  } else if (cursor.x - mapCorner.x > viewSize.x - 1 - edgeOffset.x) {
    mapCorner.x = cursor.x - (viewSize.x - 1 - edgeOffset.x);
  }
  mapCorner.x = std::clamp(mapCorner.x, 0, maxCornerMap.x);

  if (cursor.y - mapCorner.y < edgeOffset.y) {
    mapCorner.y = cursor.y - edgeOffset.y;
  } else if (cursor.y - mapCorner.y > viewSize.y - 1 - edgeOffset.y) {
    mapCorner.y = cursor.y - (viewSize.y - 1 - edgeOffset.y);
  }
  mapCorner.y = std::clamp(mapCorner.y, 0, maxCornerMap.y);
  previousMapCorner = mapCorner;

  mapOffset = sf::Vector2f(-static_cast<float>(mapCorner.x) * tileSize.x,
                           -static_cast<float>(mapCorner.y) * tileSize.y);
  cursorOffset =
      sf::Vector2f(static_cast<float>(cursor.x - mapCorner.x) * tileSize.x,
                   static_cast<float>(cursor.y - mapCorner.y) * tileSize.y) -
      sf::Vector2f(3.0f, 3.0f);
}

void Camera::startCinematic(Coord from, Coord to, sf::Time duration) {
  cinematicFrom.x = std::clamp(from.x, 0, maxCornerMap.x);
  cinematicFrom.y = std::clamp(from.y, 0, maxCornerMap.y);
  cinematicTo.x = std::clamp(to.x, 0, maxCornerMap.x);
  cinematicTo.y = std::clamp(to.y, 0, maxCornerMap.y);

  // Téléportation si duration négative (voir update)
  cinematicDuration =
      duration > sf::Time::Zero ? duration : sf::milliseconds(1);
  cinematicElapsed = sf::Time::Zero;
  cinematicActive = true;
  cursorVisible = false;

  mapCorner = cinematicFrom;
  previousMapCorner = cinematicFrom;
  mapOffset = sf::Vector2f(-cinematicFrom.x * tileSize.x,
                           -cinematicFrom.y * tileSize.y);
}

void Camera::updateCinematic(sf::Time deltaTime) {
  if (!cinematicActive) {
    return;
  }

  cinematicElapsed += deltaTime;

  float ratio = std::clamp(
      cinematicElapsed.asSeconds() / cinematicDuration.asSeconds(), 0.f, 1.f);
  // Unique polynome de plus bas degré de dérivé nulle en 0 et 1 valant 0 en 0
  // et 1 en 1
  float eased = ratio * ratio * (3.f - 2.f * ratio);

  float cornerX = cinematicFrom.x + (cinematicTo.x - cinematicFrom.x) * eased;
  float cornerY = cinematicFrom.y + (cinematicTo.y - cinematicFrom.y) * eased;

  mapOffset = sf::Vector2f(-cornerX * tileSize.x, -cornerY * tileSize.y);

  if (ratio >= 1.f) {
    cinematicActive = false;
    cursorVisible = true;
    mapCorner = cinematicTo;
    previousMapCorner = cinematicTo;
  }
}
