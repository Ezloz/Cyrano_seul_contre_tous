#include "TurnQueue.h"

#include <iostream>

void bubbleSingleSort(std::vector<std::pair<Character*, float>>& vec, int index) {
  if (index <= 0 || index <= vec.size()-1){
    return;
  }
  if (vec[index-1].second > vec[index].second){
    swap(vec[index], vec[index-1]);
    return bubbleSingleSort(vec, index-1);
  }
  if (vec[index].second > vec[index+1].second){
    swap(vec[index], vec[index+1]);
    return bubbleSingleSort(vec, index+1);
  }
}


void TurnQueue::SetQueue(const std::vector<std::pair<Character*, float>>& tQ) {
  this->turnQueue = tQ;
  std::ranges::sort(turnQueue, {}, &std::pair<Character*, float>::second);
  EndCurrentCharacter();
/*
  std::cout << turnQueue[0].first->getNameId();
  std::cout << turnQueue[1].first->getNameId();
  std::cout << turnQueue[2].first->getNameId();
  std::cout << turnQueue[0].second;
  std::cout << turnQueue[1].second;
  std::cout << turnQueue[2].second;
*/

}


int TurnQueue::FindCharacterIndex(const Character& character){
  for (int i = 0; i < this->turnQueue.size(); i++){
   if (*(turnQueue[i].first) == character){
      return i;
    }
  }

  return -1;
}
void TurnQueue::SetActionValue(int index, float actionvalue){
  turnQueue[index].second = actionvalue;
  bubbleSingleSort(turnQueue, index);
}

void TurnQueue::UpdateCurrentCharacter(float actionvalue){ // same as using setActionValue() on CurrentCharacter
  SetActionValue(currentCharacterIndex, actionvalue);
}

void TurnQueue::EndCurrentCharacter(){ //End turn of current character and select a new current character (the one with lowest AV)
  currentCharacterIndex = 0;
  currentCharacter = GetNextCharacter();
}

void TurnQueue::AddCharacter(Character& character, float actionvalue){
  Character* ch = &character;
  turnQueue.push_back(std::pair{ch, actionvalue});
  bubbleSingleSort(turnQueue, turnQueue.size()-1);
}

void TurnQueue::RemoveCharacter(const Character& character){
  int c = FindCharacterIndex(character);
  if (c == -1){
    return;
  } 
  turnQueue.erase(turnQueue.begin()+c);

}
void TurnQueue::SetActionValue(const Character& character, float actionvalue){
  SetActionValue(FindCharacterIndex(character), actionvalue);
}

//constexpr int PORTRAIT_WIDTH_CUT = 500;
//constexpr int PORTRAIT_HEIGHT_CUT = 200;
//    sprite.setTextureRect(sf::IntRect(sf::Vector2i{0, 0}, sf::Vector2i{PORTRAIT_WIDTH_CUT, PORTRAIT_HEIGHT_CUT}));


void TurnQueue::draw(sf::RenderTarget &target, sf::RenderStates states) const{
    sf::Sprite sprite{currentCharacter->getPortrait()};
    sprite.setScale({0.125f, 0.125f});
    auto height = sprite.getGlobalBounds().size.y;

    sprite.setPosition({0.f, 0.f});
    target.draw(sprite, states);
  int index = 1;
  for (auto [charac, av] : turnQueue){
    if (charac == currentCharacter && currentCharacter->isPlayer()){
      sprite.setPosition({0.f, 0.f + index * height});
      sprite.setColor(sf::Color(255, 255, 255, 125));
      target.draw(sprite, states);
    }
    else{
      sprite.setPosition({0.f, 0.f + index * height});
      target.draw(sprite, states);
    }
    index++;
  }
}

