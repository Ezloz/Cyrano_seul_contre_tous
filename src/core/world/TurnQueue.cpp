#include "world/TurnQueue.h"

#include <SFML/Graphics.hpp>
#include <iostream>

void bubbleSingleSort(std::vector<std::pair<Character *, float>> &vec,
                      int index) {
  if (index < 0 || index > vec.size() - 1) {
    return; // POSSIBLE REWORK : Maybe return an error ?
  }
  if (index > 0 && vec[index - 1].second > vec[index].second) {
    swap(vec[index], vec[index - 1]);
    return bubbleSingleSort(vec, index - 1);
  }
  if (index < vec.size() - 1 && vec[index].second > vec[index + 1].second) {
    swap(vec[index], vec[index + 1]);
    return bubbleSingleSort(vec, index + 1);
  }
}

Character *TurnQueue::GetNextCharacter() {
  if (turnQueue.empty())
    return nullptr;

  float av_to_substract = turnQueue[0].second;
  std::cout << "av :" << av_to_substract;

  for (auto &elem : turnQueue) {
    elem.second -= av_to_substract;
  }

  return turnQueue[0].first;
}

void TurnQueue::SetQueue(const std::vector<std::pair<Character *, float>> &tQ) {
  this->turnQueue = tQ;
  std::ranges::sort(turnQueue, {}, &std::pair<Character *, float>::second);
  currentCharacter = GetNextCharacter();
}

int TurnQueue::FindCharacterIndex(const Character *character) {
  for (int i = 0; i < this->turnQueue.size(); i++) {
    if (turnQueue[i].first == character) {
      return i;
    }
  }
  assert(false);
  return -1;
}
void TurnQueue::SetActionValue(int index, float actionvalue) {

  turnQueue[index].second = actionvalue;
  bubbleSingleSort(turnQueue, index);
}

void TurnQueue::AddActionValue(Character *character, float actionvalue) {
  turnQueue[FindCharacterIndex(character)].second += actionvalue;
}

void TurnQueue::UpdateCurrentCharacter(
    float actionvalue) { // same as using setActionValue() on CurrentCharacter
  SetActionValue(FindCharacterIndex(currentCharacter), actionvalue);
}

void TurnQueue::EndCurrentCharacter() { // End turn of current character and
                                        // select a new current character (the
                                        // one with lowest AV)
  UpdateCurrentCharacter(currentCharacter->getUsedAV());
  currentCharacter->resetUsedAV();
  auto av = turnQueue[0].second;
  currentCharacter = GetNextCharacter();
  std::cout << currentCharacter->getNameId() << '+' << turnQueue[0].first->getNameId() << ',' << av;
}

void TurnQueue::AddCharacter(Character *character, float actionvalue) {
  turnQueue.push_back(std::pair{character, actionvalue});
  bubbleSingleSort(turnQueue, turnQueue.size() - 1);
}

void TurnQueue::RemoveCharacter(const Character *character) {
  int c = FindCharacterIndex(character);
  if (c == -1) {
    return;
  }
  turnQueue.erase(turnQueue.begin() + c);
}
void TurnQueue::SetActionValue(const Character *character, float actionvalue) {
  SetActionValue(FindCharacterIndex(character), actionvalue);
}

// constexpr int PORTRAIT_WIDTH_CUT = 500;
// constexpr int PORTRAIT_HEIGHT_CUT = 200;
//     sprite.setTextureRect(sf::IntRect(sf::Vector2i{0, 0},
//     sf::Vector2i{PORTRAIT_WIDTH_CUT, PORTRAIT_HEIGHT_CUT}));

void TurnQueue::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  sf::Sprite sprite{currentCharacter->getPortraitSprite()};
  sprite.setScale({0.125f, 0.125f});
  auto height = sprite.getGlobalBounds()
                    .size.y; // assuming all characters portraits are same size
  sprite.setPosition({0.f, 0.f});
  sf::RectangleShape border{sprite.getGlobalBounds().size};

  border.setOutlineColor(sf::Color::White);
  border.setOutlineThickness(0.5f);
  border.setFillColor(sf::Color{255,255,255,0});

  if (currentCharacter->getIsCursorSelected()) {
    border.setPosition({0.f, 0.f});
    target.draw(border, states);
  }

  target.draw(sprite, states);
  int index = 1;
  for (auto [charac, av] : turnQueue) {
    sprite = charac->getPortraitSprite();
    sprite.setScale({0.125f, 0.125f});
    if (*charac == *currentCharacter) {
      sprite.setColor(sf::Color(255, 255, 255, 125));
      target.draw(sprite, states);
    }
    if (charac->getIsCursorSelected()) {
      border.setPosition({0.0f, index * height});
      target.draw(border, states);
    }
    sprite.setPosition({0.f, 0.f + index * height});
    target.draw(sprite, states);
    index++;
  }
}
