struct Camera {
private:
  std::int32_t cursorX;
  std::int32_t cursorY;
  std::int32_t previousCursorX;
  std::int32_t previousCursorY;
  const std::int32_t edgeOffsetX;
  const std::int32_t edgeOffsetY;
  const std::int32_t cameraSizeX;
  const std::int32_t cameraSizeY;

  sf::Time inputDuration[NB_INPUT];
  sf::Time repeatRate;
  sf::Time repeatDelay;
  sf::Time lastMove;
  bool isPressed[NB_INPUT];
  bool delayedMove;
  bool newMove;

public:
  sf::Vector2f processNewOffset(std::vector<Input> inputs);
}
