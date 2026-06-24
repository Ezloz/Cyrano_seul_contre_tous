#include "Character.h"

#include <algorithm>
#include <cassert>
#include <cfenv>
#include <cmath>
#include <cstdlib>
#include <math.h>

namespace {
constexpr float lungeReach = 0.4f;
constexpr float pi = 3.14f;
} // namespace

void Character::update(sf::Time dt) {
  if (moving) {
    moveElapsed += dt;
    if (moveElapsed >= moveDuration) {
      moving = false;
    }
  }

  if (lunging) {
    lungeElapsed += dt;
    if (lungeElapsed >= lungeDuration) {
      lunging = false;
      if (lungeTarget != nullptr) {
        processDommage(*lungeTarget);
        lungeTarget = nullptr;
      }
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
  if (coords.empty()) {
    throw std::runtime_error("moveTo effectué avec un path vide");
  }
  coord = coords.back();
  movePath = std::move(coords);
  moveTileRate = tileRate;
  moveIndexTile = 0;
  moveElapsed = sf::Time::Zero;
  const int segmentCount = static_cast<int>(movePath.size()) - 1;
  moveDuration = tileRate * static_cast<float>(segmentCount);
  moving = segmentCount >= 1;
}

void Character::lungeAt(Character &other, sf::Time duration) {
  const int dx = other.coord.x - coord.x;
  const int dy = other.coord.y - coord.y;

  const float len = std::sqrt(static_cast<float>(dx * dx + dy * dy));
  // Direction du lunge
  lungeDir =
      len > 0.f ? sf::Vector2f{dx / len, dy / len} : sf::Vector2f{0.f, 0.f};

  if (std::abs(dx) >= std::abs(dy)) {
    lungeState = dx >= 0 ? "walkRight" : "walkLeft";
  } else {
    lungeState = dy >= 0 ? "walkDown" : "walkUp";
  }

  lungeTarget = &other;
  lungeElapsed = sf::Time::Zero;
  lungeDuration = duration;
  lunging = duration > sf::Time::Zero;
}

void Character::processDommage(Character &other) {
  // TODO: appliquer les dégâts
  (void)other;
}

int Character::currentSegment(float &t) const {
  const int segmentCount = static_cast<int>(movePath.size()) - 1;

  int i = static_cast<int>(std::floor(moveElapsed / moveTileRate));
  if (i >= segmentCount) {
    t = 1.f;
    return segmentCount - 1;
  }

  t = std::fmod(moveElapsed.asMilliseconds(), moveTileRate.asMilliseconds()) /
      static_cast<float>(moveTileRate.asMilliseconds());
  return i;
}

sf::Vector2f Character::currentVisualTile() const {
  sf::Vector2f base = {static_cast<float>(coord.x),
                       static_cast<float>(coord.y)};
  if (moving && movePath.size() >= 2 && moveTileRate > sf::Time::Zero) {
    float t = 0.f;
    const int i = currentSegment(t);
    const Coord a = movePath[i];
    const Coord b = movePath[i + 1];
    base = {static_cast<float>(a.x) + static_cast<float>(b.x - a.x) * t,
            static_cast<float>(a.y) + static_cast<float>(b.y - a.y) * t};
  }

  if (lunging && lungeDuration > sf::Time::Zero) {
    const float lt = std::clamp(lungeElapsed / lungeDuration, 0.f, 1.f);
    const float reach = std::sin(pi * lt) * lungeReach;
    base.x += lungeDir.x * reach;
    base.y += lungeDir.y * reach;
  }

  return base;
}

std::string Character::movementState() const {
  if (lunging) {
    return lungeState;
  }
  if (!moving || movePath.size() < 2 || moveTileRate <= sf::Time::Zero) {
    return "idle";
  }
  float t = 0.f;
  const int i = currentSegment(t);
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
  // Center horizontally on the tile; align the sprite's bottom edge with the
  // tile's bottom edge (feet on the tile).
  const float tileW = static_cast<float>(tileSize.x);
  const float tileH = static_cast<float>(tileSize.y);
  frameSprite.setPosition(
      {tile.x * tileW + (tileW - static_cast<float>(f.w)) / 2.f,
       tile.y * tileH + (tileH - static_cast<float>(f.h))});
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
        {"maxLife", stats.maxLife},
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
  const auto assign = [&s](const std::string &key, int value) {
    if (key == "life")
      s.life = value;
    else if (key == "maxLife")
      s.maxLife = value;
    else if (key == "speed")
      s.speed = value;
    else if (key == "charisma")
      s.charisma = value;
    else if (key == "power")
      s.power = value;
    else if (key == "luck")
      s.luck = value;
    else if (key == "range")
      s.range = value;
  };

  if (j.is_array()) {
    for (const auto &entry : j) {
      for (const auto &[key, value] : entry.items()) {
        assign(key, value.get<int>());
      }
    }
  } else if (j.is_object()) {
    for (const auto &[key, value] : j.items()) {
      assign(key, value.get<int>());
    }
  }
  return s;
}
