// TODO:: Json parseur pour sauvegarder l'état des characters + importer d'état
// des ennemis par défaut
#pragma once

#include "Animation.h"
#include "GameTypes.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <tmxlite/Map.hpp>
#include <tmxlite/Types.hpp>

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// struct Object {};
// struct Weapon {};
// struct Armor {};
// struct Accessory {};

class Character {
public:
  struct Statistic {
    int life = 1, speed = 1, charisma = 1, power = 1, luck = 1, range = 1;
  };

private:
  const AnimationTemplate *animTemplate;
  std::shared_ptr<sf::Texture> texture;
  sf::Sprite sprite;
  std::shared_ptr<sf::Texture> portrait;
  std::string state = "idle";
  std::size_t frameIndex = 0;
  sf::Time elapsed = sf::Time::Zero;

  std::vector<Coord> movePath;
  sf::Time moveElapsed = sf::Time::Zero;
  sf::Time moveTileRate = sf::Time::Zero;
  sf::Time moveDuration = sf::Time::Zero;
  int moveIndexTile = 0;
  bool moving = false;
  bool isCursorSelected = false;

  bool lunging = false;
  sf::Time lungeElapsed = sf::Time::Zero;
  sf::Time lungeDuration = sf::Time::Zero;
  sf::Vector2f lungeDir = {0.f, 0.f};
  std::string lungeState = "idle";
  // Cible de l'attaque en cours (dois etre enregistrer pour persister entre
  // frame)
  Character *lungeTarget = nullptr;

  // Position du sprite, peut etre entre deux tiles
  sf::Vector2f currentVisualTile() const;

  // indice actuel dans le movePath et met à jour t (pourcentage du parcours sur
  // la tile, utiliser pour la position du sprite)
  int currentSegment(float &t) const;

  // renvoie la direction associé au mouvement actuel ("idle", "walkUp",
  // "walkDown", "walkLeft" or "walkRight").
  std::string movementState() const;

  // Appelé une fois l'animation d'attaque (lunge) terminée. Appliquera les
  // dégâts à `other` ; ne fait rien pour l'instant.
  void processDommage(Character &other);

protected:
  float usedAV;
  Coord coord;
  std::optional<std::string> specialAttackName;
  Statistic stats;
  std::string nameId;
  std::string type;
  std::vector<std::string> effectIds;
  std::vector<std::string> equipementIds;
  // std::optional<Weapon> weapon;
  // std::optional<Armor> armor;
  // std::optional<Accessory> firstAccessory;
  // std::optional<Accessory> secondAccessory;

public:
  Character(std::string nameId, std::string type, Coord coord,
            std::optional<std::string> specialAttackName, Statistic stats,
            std::vector<std::string> effectIds,
            std::vector<std::string> equipementIds,
            const AnimationTemplate *tmpl, std::shared_ptr<sf::Texture> texture,
            std::shared_ptr<sf::Texture> portrait)
      : animTemplate(tmpl), texture(std::move(texture)), sprite(*this->texture),
        portrait(portrait), coord(coord),
        specialAttackName(std::move(specialAttackName)), stats(stats),
        nameId(std::move(nameId)), type(std::move(type)),
        effectIds(std::move(effectIds)),
        equipementIds(std::move(equipementIds)) {
    state = animTemplate->defaultState();
  }
  virtual ~Character() = default;

  bool operator==(const Character &other) const {
    return getNameId() == other.getNameId();
  }

  virtual void attack(Character &other) = 0;
  virtual void specialAttack(Character &other) = 0;
  virtual bool isPlayer() const = 0;
  virtual bool
  workAI(const std::vector<size_t> &walkableGrid, const int gridWidth,
         const int gridHeight,
         const std::vector<std::unique_ptr<Character>> &characters) = 0;

  virtual std::vector<Coord>
  calculateMoveRange(const std::vector<size_t> &walkableGrid,
                     const int gridWidth, const int gridHeight,
                     const std::vector<std::unique_ptr<Character>> &characters);

  float getUsedAV() { return this->usedAV; }
  void resetUsedAV() { this->usedAV = 0.0f; }

  virtual json toJson() const;
  // Utiliser pour save.json et non la saveMap.json
  json toPartyJson() const;
  static Statistic statisticFromJson(const json &j);

  void update(sf::Time dt);
  void draw(sf::RenderTarget &target, const tmx::Vector2u &tileSize,
            sf::RenderStates states = {}) const;

  void moveTo(std::vector<Coord> coords, sf::Time tileRate);

  void lungeAt(Character &other, sf::Time duration = sf::milliseconds(300));

  void setState(const std::string &state) {
    this->state = state;
    frameIndex = 0;
    elapsed = sf::Time::Zero;
  }
  bool getIsCursorSelected() { return isCursorSelected; }
  void setIsCursorSelected(bool f) { this->isCursorSelected = f; }
  sf::Sprite getPortraitSprite() const { return sf::Sprite{*portrait}; }
  Coord getCoord() const { return coord; }
  void setCoord(Coord c) { coord = c; }
  const std::string &getNameId() const { return nameId; }
  const std::string &getType() const { return type; }
  const Statistic &getStats() const { return stats; }
  const std::optional<std::string> &getSpecialAttackName() const {
    return specialAttackName;
  }
};
