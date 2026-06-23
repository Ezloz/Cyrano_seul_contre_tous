#pragma once

#include "Character.h"

class PlayerCharacter : public Character {
public:
  PlayerCharacter(std::string nameId, std::string type, Coord coord,
                  std::optional<std::string> specialAttackName, Statistic stats,
                  std::vector<std::string> effectIds,
                  std::vector<std::string> equipementIds,
                  const AnimationTemplate *tmpl,
                  std::shared_ptr<sf::Texture> texture,
                  std::shared_ptr<sf::Texture> portrait)
      : Character(std::move(nameId), std::move(type), coord,
                  std::move(specialAttackName), stats, std::move(effectIds),
                  std::move(equipementIds), tmpl, std::move(texture),
                  std::move(portrait)) {}

  bool isPlayer() const override { return true; }
  bool workAI(const std::vector<size_t>& walkableGrid, const int gridWidth, const int gridHeight,
              const std::vector<std::unique_ptr<Character>>& characters) {return false;}

  // Tous les childrens doivent implémenter :
  // void attack(Character &other) override;
  // void specialAttack(Character &other) override;
};
