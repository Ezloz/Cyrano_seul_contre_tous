#pragma once
#include <vector>
#include <algorithm>

#include "entities/Character.h"

constexpr float BASE_DEFAULT_AV = 10000.0f;


class TurnQueue : public sf::Drawable {
private:
  Character* currentCharacter;

  /*   
  MUST BE SORTED AT ALL TIME 
  Any modification to this attribute NEED to be followed by a sort  of this attribute;
  */ 
  std::vector<std::pair<Character*, float>> turnQueue; 

  int FindCharacterIndex(const Character* character);
  void SetActionValue(int index, float actionvalue);
  Character* NextCharacter();

public:
  int GetSize() const {return turnQueue.size();};
  void SetQueue(const std::vector<std::pair<Character*, float>>& tQ);
  Character* GetCurrentCharacter() {return currentCharacter;};
  void AddActionValue(Character* character, float actionvalue);
  void UpdateCurrentCharacter(float actionvalue); // same as using setActionValue() on CurrentCharacter
  void EndCurrentCharacter(); //End turn of current character and select a new current character (the one with lowest AV)
  void AddCharacter(Character* character, float actionvalue);
  void RemoveCharacter(const Character* character);
  void SetActionValue(const Character* character, float actionvalue);

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

};