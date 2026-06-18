// TODO:: Json parseur pour sauvegarder l'état des characters + importer d'état
// des ennemis par défaut
#pragma once

#include "Animation.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct Object {};
struct Weapon {};
struct Armor {};
struct Accessory {};

class Character {
public:
  struct Statistic {
    int life = 0, speed = 0, charisma = 0, power = 0, luck = 0, range = 0;
  };

private:
  const AnimationTemplate *animTemplate;
  std::shared_ptr<sf::Texture> texture;
  sf::Sprite sprite;
  std::string state = "idle";
  std::size_t frameIndex = 0;
  sf::Time elapsed = sf::Time::Zero;

protected:
  std::int32_t coordX = 0;
  std::int32_t coordY = 0;
  std::string specialAttackName;
  Statistic stats;
  std::vector<Object> inventory;
  std::optional<Weapon> weapon;
  std::optional<Armor> armor;
  std::optional<Accessory> firstAccessory;
  std::optional<Accessory> secondAccessory;

public:
  Character(std::int32_t x, std::int32_t y, std::string specialAttackName,
            const AnimationTemplate *tmpl, std::shared_ptr<sf::Texture> texture)
      : animTemplate(tmpl), texture(std::move(texture)), sprite(*this->texture),
        coordX(x), coordY(y), specialAttackName(specialAttackName) {
    state = animTemplate->defaultState();
  }
  virtual ~Character() = default;
  virtual void attack(Character &other) = 0;
  virtual void specialAttack(Character &other) = 0;

  void update(sf::Time dt);
  void draw(sf::RenderTarget &target, const tmx::Vector2u &tileSize) const;

  void setState(const std::string &state) {
    this->state = state;
    frameIndex = 0;
    elapsed = sf::Time::Zero;
  }
  std::int32_t getCoordX() const { return coordX; }
  std::int32_t getCoordY() const { return coordY; }
  void setCoordX(std::int32_t x) { coordX = x; }
  void setCoordY(std::int32_t y) { coordY = y; }
  const Statistic &getStats() const { return stats; }
  const std::string &getSpecialAttackName() const { return specialAttackName; }
};
