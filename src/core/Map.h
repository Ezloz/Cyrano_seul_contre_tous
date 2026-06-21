#pragma once
#include "Camera.h"
#include "Character.h"
#include "GameTypes.h"
#include "MapLayer.h"
#include "TurnQueue.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tmxlite/ObjectGroup.hpp>
#include <vector>

class Camera;

struct MapExit {
  std::string name;
  Coord coord;
  std::vector<std::string> directions;
  std::string targetMapId;
  std::optional<std::string> targetEntry;
};

class Map : public sf::Drawable {
private:
  tmx::Map tmxMap;
  std::unique_ptr<Camera> activeCamera;
  std::vector<std::unique_ptr<MapLayer>> layers;
  std::unique_ptr<MapLayer> cursorLayer;
  tmx::Map cursorTmx;
  tmx::Vector2u tileSize;
  Coord viewSize = {15, 10};
  Coord edgeOffset = {3, 2};

  std::string mapId;
  std::vector<std::unique_ptr<Character>> characters;
  std::vector<MapExit> exits;

  int gridWidth = 0;
  int gridHeight = 0;
  std::vector<char> walkableGrid;
  void computeWalkableGrid();

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    for (const auto &layer : layers) {
      target.draw(*layer, states);
    }
    sf::RenderStates characterStates = states;
    characterStates.transform.translate(activeCamera->getMapOffset());
    for (const auto &character : characters) {
      character->draw(target, tileSize, characterStates);
    }
    if (cursorLayer && activeCamera->isCursorVisible()) {
      target.draw(*cursorLayer, states);
    }
  }

public:
  explicit Map(const std::string &name, int nbLayer = 1);
  ~Map();

  tmx::Vector2u GetTileSize() const { return tileSize; };

  Coord GetViewSize() const { return viewSize; };

  bool isWalkable(Coord coord) const;
  void setWalkable(Coord coord, bool walkable);

  void setMapId(const std::string &id) { mapId = id; }
  void addCharacter(std::unique_ptr<Character> character) {
    characters.push_back(std::move(character));
  }
  void addExit(MapExit exit) { exits.push_back(std::move(exit)); }
  const std::vector<MapExit> &getExits() const { return exits; }

  // Le premier Coord doit coincider avec la position initiale du character
  void moveCharacterTo(const std::string &nameId, std::vector<Coord> path,
                       sf::Time tileRate) {
    for (auto &character : characters) {
      if (character->getNameId() == nameId) {
        character->moveTo(std::move(path), tileRate);
        return;
      }
    }
  }

  void move(std::set<Input> inputs, std::set<Input> inputsRelease,
            sf::Time deltaTime);

  void startCinematic(Coord from, Coord to, sf::Time duration);
  bool isCinematicActive() const;

  void update(sf::Time elapsed);

  // Persiste l'état de la map dans le slot donné
  // (saves/slot{N}/Maps/{mapId}/mapSave.json).
  void saveState(int N) const;

  // Charge la map mapId du slot. Si aucune sauvegarde n'existe pour cette map
  // dans ce slot, génère l'état par défaut
  // (resources/Maps/{mapId}/defaultMap.json) et l'écrit dans le slot.
  static std::unique_ptr<Map> loadMap(int slot, const std::string &mapId);
};
