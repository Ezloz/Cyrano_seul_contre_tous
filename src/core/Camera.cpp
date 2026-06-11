sf::Vector2f Camera::processNewOffset(std::set<Input> inputs,
                                      std::set<Input> releaseInputs,
                                      sf::Time lastMove = 0) {

  sf::Vector2f newOffset = sf::Vector2f(0.f, 0.f);
  bool updateCursor = false;
  lastMove += GameApp::getInstance().getDeltaTime();
  if (delayedMove && lastMove >= repeatDelay) {
    delayedMove = false;
    lastMove = lastMove % repeatDelay;
    previousCursorX = cursorX;
    previousCursorY = cursorY;
    updateCursor = true;
  } else if (lastMove >= rateDelay) {
    lastMove = lastMove % rateDelay;
    updateCursor = true;
  }

  for (const &auto release : releaseInputs) {
    switch (release) {
    case UP:
      isPressed[UP] = false;
      delayedMove = true;
      break;
    case DOWN:
      isPressed[DOWN] = false;
      delayedMove = true;
      break;
    case LEFT:
      isPressed[LEFT] = false;
      delayedMove = true;
      break;
    case RIGHT:
      isPressed[RIGHT] = false;
      delayedMove = true;
      break;
    }
  }

  for (const &auto input : inputs) {
    switch (input) {
    case UP:
      if (inputs.contains(DOWN)) {
        isPressed[UP] = false;
        break;
      }
      if (cursorY <= 0) {
        break;
      }
      if (!isPressed[UP]) {
        delayedMove = true;
        isPressed[UP] = true;
      }
      if (updateCursor) {
        cursorY -= 1;
      }
      break;

    case DOWN:
      if (inputs.contains(UP)) {
        isPressed[DOWN] = false;
        break;
      }
      if (cursorY >= mapSizeY) {
        break;
      }
      if (!isPressed[DOWN]) {
        delayedMove = true;
        isPressed[DOWN] = true;
      }
      if (updateCursor) {
        cursorY += 1;
      }
      break;

    case LEFT:
      if (inputs.contains(RIGHT)) {
        isPressed[LEFT] = false;
        break;
      }
      if (cursorX <= 0) {
        break;
      }
      if (!isPressed[LEFT]) {
        delayedMove = true;
        isPressed[LEFT] = true;
      }
      if (updateCursor) {
        cursorX -= 1;
      }
      break;

    case RIGHT:
      if (inputs.contains(LEFT)) {
        isPressed[RIGHT] = false;
        break;
      }
      if (cursorX >= mapSizeX) {
        break;
      }
      if (!isPressed[RIGHT]) {
        delayedMove = true;
        isPressed[RIGHT] = true;
      }
      if (updateCursor) {
        cursorX += 1;
      }
      break;
    }
  }

  if (delayedMove && lastMove <= rateDelay) {
    sf::Vector2f newOffset =
        sf::Vector2f(cursorX.to_pixel(), cursorY.to_pixel())
  } else {
    sf::Vector2f newOffset = sf::Vector2f(
        (cursorX - previousCursorX).to_pixel() * lastMove / rateDelay +
            previousCursorX.to_pixel(),
        (cursorY - previousCursorY).to_pixel() * lastMove / rateDelay +
            previousCursorY.to_pixel())
  }
  return newOffset;
}
