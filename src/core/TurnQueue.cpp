#include "TurnQueue.h"

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

void TurnQueue::draw(){
  for (auto [charac, av] : turnQueue){
    if (charac == currentCharacter){
      //set alpha value to 0.5
    }

  }
}

void drawRectangleWithEnemyPortraitAndAV(float x, float y, float width, float height){

}