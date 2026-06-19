#pragma once
#include <vector>
#include <algorithm>

#include "Character.h"



class TurnQueue : public sf::Drawable {
private:
  Character* currentCharacter;
  int currentCharacterIndex;

  /*   
  MUST BE SORTED AT ALL TIME 
  Any modification to this attribute NEED to be followed by a sort  of this attribute;
  */ 
  std::vector<std::pair<Character*, float>> turnQueue; 

  int FindCharacterIndex(const Character& character);
  void SetActionValue(int index, float actionvalue);

public:
  explicit TurnQueue(const std::vector<std::pair<Character*, float>>& tQ) : turnQueue{tQ} {
    std::ranges::sort(turnQueue, {}, &std::pair<Character*, float>::second);
    EndCurrentCharacter();
    }

  Character* GetCurrentCharacter() {return currentCharacter;};
  Character* GetNextCharacter() {return turnQueue[0].first;};
  void UpdateCurrentCharacter(float actionvalue); // same as using setActionValue() on CurrentCharacter
  void EndCurrentCharacter(); //End turn of current character and select a new current character (the one with lowest AV)
  void AddCharacter(Character& character, float actionvalue);
  void RemoveCharacter(const Character& character);
  void SetActionValue(const Character& character, float actionvalue);

  void draw();

};