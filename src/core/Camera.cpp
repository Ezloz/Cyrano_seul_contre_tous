#include "Camera.h"
#include <algorithm>
#include <set>

sf::Vector2f Camera::processNewOffset(std::set<Input> inputs,
                                      std::set<Input> releaseInputs) {
  const int UP = static_cast<int>(Input::UP);
  const int DOWN = static_cast<int>(Input::DOWN);
  const int LEFT = static_cast<int>(Input::LEFT);
  const int RIGHT = static_cast<int>(Input::RIGHT);

  bool freshPress = false;
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

  std::int32_t dx = (isPressed[RIGHT] ? 1 : 0) - (isPressed[LEFT] ? 1 : 0);
  std::int32_t dy = (isPressed[DOWN] ? 1 : 0) - (isPressed[UP] ? 1 : 0);

  this->lastMove += GameApp::GetInstance()->GetDeltaTime();

  bool doStep = false;
  if (dx != 0 || dy != 0) {
    if (freshPress) {
      doStep = true;
      delayedMove = true;
      this->lastMove = sf::Time::Zero;
    } else if (delayedMove && this->lastMove >= repeatDelay) {
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
    cursorX = std::clamp(cursorX + dx, 0, mapSizeX);
    cursorY = std::clamp(cursorY + dy, 0, mapSizeY);
  }

  for (const auto &release : releaseInputs) {
    isPressed[static_cast<int>(release)] = false;
  }

  // Once the current step's animation has run its course the cursor is settled,
  // so pin the previous tile to the current one (this also covers the idle
  // case, where lastMove keeps growing past repeatRate).
  if (this->lastMove >= repeatRate) {
    previousCursorX = cursorX;
    previousCursorY = cursorY;
  }

  float ratio =
      std::min(1.f, this->lastMove.asSeconds() / repeatRate.asSeconds());
  float offsetX = (previousCursorX + (cursorX - previousCursorX) * ratio) *
                  (float)tileSize.x;
  float offsetY = (previousCursorY + (cursorY - previousCursorY) * ratio) *
                  (float)tileSize.y;
  return sf::Vector2f(offsetX, offsetY);
}
