#pragma once

#include "PlayerCharacter.h"
#include "Save.h"

#include <memory>
#include <string>
#include <vector>

class Cyrano : public PlayerCharacter {
public:
  Cyrano(std::string nameId, Coord coord, Statistic stats,
         std::vector<std::string> effectIds,
         std::vector<std::string> equipementIds);

  void attack(Character &other) override;
  void specialAttack(Character &other) override;

  // Factory
  static std::unique_ptr<Character> create(const json &j);
};
