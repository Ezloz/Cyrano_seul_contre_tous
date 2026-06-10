struct Camera {
private:
  sf::Time inputDuration[NB_INPUT];
  sf::Time tresholdMouvement;
  sf::Time firstTresholdMouvement;
  bool firstMoves[NB_INPUT];
  bool firstMove;

public:
  sf::Vector2f processNewOffset(std::vector<Input> inputs);
}
