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
  const std::int32_t tileSize;

  sf::Time repeatRate;
  sf::Time repeatDelay;
  sf::Time lastMove;
  bool isPressed[NB_INPUT];
  bool delayedMove;

public:
  Camera Camera(std::int32_t initialCursorX, std::int32_t initialCursorY,
                std::int32_t edgeOffsetX, const std::int32_t edgeOffsetY,
                tmx::Map &map) {
    cursorX = initialCursorX;
    cursorY = initialCursorY;
    previousCursorX = initialCursorX;
    previousCursorY = initialCursorY;
    edgeOffsetX = 5;
    edgeOffsetY = 5;
    cameraSizeX = 10;
    cameraSizeY = 10;
    tileSize = map.tileSize;
    repeatRate = sf::milliseconds(80);
    repeatDelay = sf::milliseconds(200);
    isPressed = {false};
    delayedMove = true;
  }

  sf::Vector2f processNewOffset(std::vector<Input> inputs);
}
