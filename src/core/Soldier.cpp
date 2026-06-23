#include "Soldier.h"

#include <optional>

namespace {
constexpr const char *typeId = "Soldier";
constexpr const char *portraitId = "Soldier";
constexpr const char *spriteId = "Soldier";
} // namespace

Soldier::Soldier(std::string nameId, Coord coord, Statistic stats,
                 std::vector<std::string> effectIds,
                 std::vector<std::string> equipementIds)
    : NPC(std::move(nameId), typeId, coord, std::nullopt, stats,
          std::move(effectIds), std::move(equipementIds),
          getAnimationTemplate(spriteId), getTexture(spriteId),
          getPortrait(portraitId)) {}

void Soldier::attack(Character &other) {}
void Soldier::specialAttack(Character &other) {}

std::unique_ptr<Character> Soldier::create(const json &j) {
  return std::make_unique<Soldier>(
      j.at("nameId").get<std::string>(),
      Coord{j.at("coord").at("x").get<int>(), j.at("coord").at("y").get<int>()},
      Character::statisticFromJson(j.at("statistics")),
      j.value("effectIds", std::vector<std::string>{}),
      j.value("equipementIds", std::vector<std::string>{}));
}
