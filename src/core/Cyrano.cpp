#include "Cyrano.h"

namespace {
constexpr const char *typeId = "Cyrano";
constexpr const char *spriteId = "Cyrano";
constexpr const char *specialAttackId = "Panache";
} // namespace

Cyrano::Cyrano(std::string nameId, Coord coord, Statistic stats,
               std::vector<std::string> effectIds,
               std::vector<std::string> equipementIds)
    : PlayerCharacter(std::move(nameId), typeId, coord, specialAttackId, stats,
                      std::move(effectIds), std::move(equipementIds),
                      getAnimationTemplate(spriteId), getTexture(spriteId)) {}

void Cyrano::attack(Character &other) {}
void Cyrano::specialAttack(Character &other) {}

std::unique_ptr<Character> Cyrano::create(const json &j) {
  return std::make_unique<Cyrano>(
      j.at("nameId").get<std::string>(),
      Coord{j.at("coord").at("x").get<int>(), j.at("coord").at("y").get<int>()},
      Character::statisticFromJson(j.at("statistics")),
      j.value("effectIds", std::vector<std::string>{}),
      j.value("equipementIds", std::vector<std::string>{}));
}
