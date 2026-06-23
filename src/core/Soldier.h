#pragma once

#include "NPC.h"
#include "Save.h"

#include <memory>
#include <string>
#include <vector>

class Soldier : public NPC {
public:
  Soldier(std::string nameId, Coord coord, Statistic stats,
          std::vector<std::string> effectIds,
          std::vector<std::string> equipementIds);

  void attack(Character &other) override;
  void specialAttack(Character &other) override;

  bool workAI(const std::vector<size_t>& walkableGrid, const int gridWidth, const int gridHeight,
              const std::vector<std::unique_ptr<Character>>& characters) override;

  // Factory
  static std::unique_ptr<Character> create(const json &j);
};
