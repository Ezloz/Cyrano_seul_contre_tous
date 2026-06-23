#pragma once

#include "Character.h"

class NPC : public Character {
public:
  NPC(std::string nameId, std::string type, Coord coord,
      std::optional<std::string> specialAttackName, Statistic stats,
      std::vector<std::string> effectIds,
      std::vector<std::string> equipementIds, const AnimationTemplate *tmpl,
      std::shared_ptr<sf::Texture> texture,
      std::shared_ptr<sf::Texture> portrait)
      : Character(std::move(nameId), std::move(type), coord,
                  std::move(specialAttackName), stats, std::move(effectIds),
                  std::move(equipementIds), tmpl, std::move(texture),
                  std::move(portrait)) {}

  bool isPlayer() const override { return false; }

  // Update les coord du NPC et son movePath. A implementer
  void move();
};
