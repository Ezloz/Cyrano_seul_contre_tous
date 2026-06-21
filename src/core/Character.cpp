#include "Character.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

void Character::update(sf::Time dt) {
  if (moving) {
    moveElapsed += dt;
    if (moveElapsed >= moveDuration) {
      moving = false;
    }
  }

  // Change l'animation en fonction de la direction du mouvement
  const std::string desiredState = movementState();
  if (desiredState != state && animTemplate->has(desiredState)) {
    setState(desiredState);
  }

  const Animation &anim = animTemplate->find(state);
  elapsed += dt;
  // Update l'animation
  while (elapsed >= anim.frames[frameIndex].duration) {
    elapsed -= anim.frames[frameIndex].duration;
    ++frameIndex;
    if (frameIndex >= anim.frames.size()) {
      frameIndex = 0;
    }
  }
}

void Character::moveTo(std::vector<Coord> coords, sf::Time tileRate) {
  if (!coords.empty()) {
    coord = coords.back();
  }
  spriteMoveTo(std::move(coords), tileRate);
}

void Character::spriteMoveTo(std::vector<Coord> coords, sf::Time tileRate) {
  int totalTiles = 0;
  for (std::size_t i = 0; i + 1 < coords.size(); ++i) {
    const Coord a = coords[i];
    const Coord b = coords[i + 1];
    assert(
        (a.x == b.x || a.y == b.y) &&
        "Le mouvement des sprite doit s'effectuer le long des axes"); // Eventuellement
                                                                      // à
                                                                      // modifier
    totalTiles += std::abs(b.x - a.x) + std::abs(b.y - a.y);
  }
  movePath = std::move(coords);
  moveTileRate = tileRate;
  moveElapsed = sf::Time::Zero;
  moveDuration = tileRate * static_cast<float>(totalTiles);
  moving = totalTiles > 0;
}

std::size_t Character::currentSegment(float &t) const {
  float tilesTraveled = moveElapsed / moveTileRate;
  for (std::size_t i = 0; i + 1 < movePath.size(); ++i) {
    const Coord a = movePath[i];
    const Coord b = movePath[i + 1];
    const float segLen =
        static_cast<float>(std::abs(b.x - a.x) + std::abs(b.y - a.y));
    const bool lastSegment = (i + 2 == movePath.size());
    if (tilesTraveled <= segLen || lastSegment) {
      t = segLen > 0.f ? std::clamp(tilesTraveled / segLen, 0.f, 1.f) : 1.f;
      return i;
    }
    tilesTraveled -= segLen;
  }
  t = 1.f;
  return 0;
}

sf::Vector2f Character::currentVisualTile() const {
  if (!moving || movePath.size() < 2 || moveTileRate <= sf::Time::Zero) {
    return {static_cast<float>(coord.x), static_cast<float>(coord.y)};
  }
  float t = 0.f;
  const std::size_t i = currentSegment(t);
  const Coord a = movePath[i];
  const Coord b = movePath[i + 1];
  return {static_cast<float>(a.x) + static_cast<float>(b.x - a.x) * t,
          static_cast<float>(a.y) + static_cast<float>(b.y - a.y) * t};
}

std::string Character::movementState() const {
  if (!moving || movePath.size() < 2 || moveTileRate <= sf::Time::Zero) {
    return "idle";
  }
  float t = 0.f;
  const std::size_t i = currentSegment(t);
  const Coord a = movePath[i];
  const Coord b = movePath[i + 1];
  if (b.x > a.x)
    return "walkRight";
  if (b.x < a.x)
    return "walkLeft";
  if (b.y > a.y)
    return "walkDown";
  if (b.y < a.y)
    return "walkUp";
  return "idle";
}

void Character::draw(sf::RenderTarget &target, const tmx::Vector2u &tileSize,
                     sf::RenderStates states) const {
  const Animation &anim = animTemplate->find(state);
  const Frame &f = anim.frames[frameIndex];

  const sf::Vector2f tile = currentVisualTile();

  sf::Sprite frameSprite = sprite;
  frameSprite.setTextureRect(sf::IntRect({f.x, f.y}, {f.w, f.h}));
  frameSprite.setPosition({tile.x * static_cast<float>(tileSize.x),
                           tile.y * static_cast<float>(tileSize.y)});
  target.draw(frameSprite, states);
}

json Character::toJson() const {
  json j = toPartyJson();
  j["nameId"] = nameId;
  j["coord"] = {{"x", coord.x}, {"y", coord.y}};
  return j;
}

json Character::toPartyJson() const {
  return json{
      {"type", type},
      {"statistics",
       {{"life", stats.life},
        {"speed", stats.speed},
        {"charisma", stats.charisma},
        {"power", stats.power},
        {"luck", stats.luck},
        {"range", stats.range}}},
      {"effectIds", effectIds},
      {"equipementIds", equipementIds},
  };
}

Character::Statistic Character::statisticFromJson(const json &j) {
  Statistic s;
  s.life = j.value("life", 0);
  s.speed = j.value("speed", 0);
  s.charisma = j.value("charisma", 0);
  s.power = j.value("power", 0);
  s.luck = j.value("luck", 0);
  s.range = j.value("range", 0);
  return s;
}
