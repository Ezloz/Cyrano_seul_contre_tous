#pragma once
#include "app/GameTypes.h"
#include "entities/Character.h"
#include "graphics/Camera.h"
#include "graphics/MapLayer.h"
#include "world/TurnQueue.h"
#include "ui/UIManager.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

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
  UIManager* uimanager = nullptr;
  std::vector<std::unique_ptr<MapLayer>> layers;
  std::unique_ptr<MapLayer> cursorLayer;
  tmx::Vector2u tileSize;
  Coord viewSize = {15, 10};
  Coord edgeOffset = {3, 2};
  TurnQueue turnQueue;
  std::vector<Coord> walkPath = {};
  std::vector<Coord> attackRange = {};
  std::vector<Coord> moveRange = {};
  sf::Texture blueTileTexture; // loaded once from dataset["blueTile"]
  sf::Texture redTileTexture; // loaded once from dataset["redTile"]
  
  std::string mapId;
  std::vector<std::unique_ptr<Character>> characters;
  std::vector<MapExit> exits;
  
  Character *selectedCharacter = nullptr;
  bool freshpress = true;
  int gridWidth = 0;
  int gridHeight = 0;
  std::vector<size_t> walkableGrid;
  void updateWalkPathAndAV();

  // Delay le mouvement
  struct PendingMove {
    std::string nameId;
    std::vector<Coord> path;
    sf::Time tileRate;
  };
  std::optional<PendingMove> pendingMove;

  void removeDeadCharacters();
  
  Character* FindCharacterByCoord(const Coord& position);
  
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
    if (selectedCharacter) {
      this->drawOverTiles(this->blueTileTexture, target, states, this->moveRange);
      this->drawOverTiles(this->redTileTexture, target, states, this->attackRange);
//    this->drawPortraitAndStats();
    }
    if (cursorLayer && activeCamera->isCursorVisible()) {
      target.draw(*cursorLayer, states);
    }
    turnQueue.draw(target, states);
  }

public:
  explicit Map(const std::string &name, int nbLayer = 1);
  ~Map();

  tmx::Vector2u GetTileSize() const { return tileSize; };

  Coord GetViewSize() const { return viewSize; };

  Character *getActiveCharacter() { return turnQueue.GetCurrentCharacter(); }

  bool isWalkable(Coord coord) const;
  void setWalkable(Coord coord, bool walkable);

  void setMapId(const std::string &id) { mapId = id; }
  void setCursor(Coord c) {
    activeCamera->setCursor(c);
    move();
  }
  void addCharacter(std::unique_ptr<Character> character) {
    characters.push_back(std::move(character));
  }
  void addExit(MapExit exit) { exits.push_back(std::move(exit)); }
  const std::vector<MapExit> &getExits() const { return exits; }

  // Le premier Coord doit coincider avec la position initiale du character
  void moveCharacterTo(const std::string &nameId, std::vector<Coord> path,
                       sf::Time tileRate) {
    if (path.empty()) {
      return;
    }
    const Coord start = path.front();
    const Coord end = path.back();
    const Coord viewSize = activeCamera->getViewSize();
    const Coord maxCorner = activeCamera->getMaxCornerMap();
    Coord middle;
    middle.x =
        std::clamp((start.x + end.x) / 2 - viewSize.x / 2, 0, maxCorner.x);
    middle.y =
        std::clamp((start.y + end.y) / 2 - viewSize.y / 2, 0, maxCorner.y);
    activeCamera->startCinematic(activeCamera->getMapCorner(), middle,
                                 sf::milliseconds(300));
    // Defer the actual move until the cinematic has finished (see update()).
    pendingMove = PendingMove{nameId, std::move(path), tileRate};
  }

  GameState ProcessInputs(std::set<Input> inputs, std::set<Input> justPressedInputs,
                          std::set<Input> inputsRelease, sf::Time deltaTime);
  void move();
  bool isCinematicActive() const;

  void update(sf::Time elapsed);

  void drawOverTiles(const sf::Texture& tile_tex, sf::RenderTarget &target, sf::RenderStates states,
                     const std::vector<Coord> &tiles) const;

  // Persiste l'état de la map dans le slot donné
  // (saves/slot{N}/Maps/{mapId}/mapSave.json).
  void saveState(int N) const;

  // Charge la map mapId du slot. Si aucune sauvegarde n'existe pour cette map
  // dans ce slot, génère l'état par défaut
  // (resources/Maps/{mapId}/defaultMap.json) et l'écrit dans le slot.
  static std::unique_ptr<Map> loadMap(UIManager* uimanager, int slot, const std::string &mapId);
};
