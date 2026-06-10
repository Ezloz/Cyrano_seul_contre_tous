sf::Vector2f Camera::processNewOffset(std::vector<Input> inputs) {
  sf::Vector2f newOffset = sf::Vector2f(0.f, 0.f);
  for (const &auto input : inputs) {
    if (newMove) {
      firstMove = false;
    }

    switch (input) {
    case UP:
      if (!isPressed[UP]) {
        firstMove = true;
        isPressed[UP] = true;
        newOffset += sf::Vector2f(1.f, 0.f);
      } else if (firstMoves[UP]) {
        firstMove = true;
        firstMoves[UP] = false;
      }
    case DOWN:
    case LEFT:
    case RIGHT:
    }
  }
}
