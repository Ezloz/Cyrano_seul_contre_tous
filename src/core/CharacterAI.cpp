#include "Soldier.h"


bool Soldier::workAI(const std::vector<size_t>& map, const int gridWidth,
                     const std::vector<std::unique_ptr<Character>>& characters){
      this->usedAV = 1000.0f / this->getStats().speed;
      return true;
}