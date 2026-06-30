#include "world/Map.h"

#include "app/GameApp.h"
#include "entities/CharacterAI.h"
#include "entities/CharacterFactory.h"
#include "io/Save.h"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Property.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Tileset.hpp>

#include <algorithm>
#include <filesystem>
#include <queue>
#include <ranges>
#include <unordered_set>

constexpr GameState DEFAULT_STATE = GameState::IN_GAME;

namespace {
std::string tmxPathForMap(const std::string &mapId) {
  json data = openJson(DATASET);
  return data.at("maps").at(mapId).get<std::string>();
}

// Finds the tileset that owns a global tile id (gid), or nullptr.
const tmx::Tileset *tilesetForGid(const tmx::Map &map, std::uint32_t gid) {
  for (const auto &tileset : map.getTilesets()) {
    if (tileset.hasTile(gid)) {
      return &tileset;
    }
  }
  return nullptr;
}

Coord coordFromJson(const json &j) {
  return {j.at("x").get<int>(), j.at("y").get<int>()};
}

json makeCreateJson(const std::string &nameId, const std::string &type,
                    Coord coord, const json &statistics, const json &effectIds,
                    const json &equipementIds) {
  return json{{"nameId", nameId},
              {"type", type},
              {"coord", {{"x", coord.x}, {"y", coord.y}}},
              {"statistics", statistics},
              {"effectIds", effectIds},
              {"equipementIds", equipementIds}};
}

MapExit exitFromJson(const std::string &name, const json &j) {
  MapExit exit;
  exit.name = name;
  exit.coord = coordFromJson(j);
  exit.directions = j.value("direction", std::vector<std::string>{});
  const json mapExit = j.value("mapExit", json::object());
  exit.targetMapId = mapExit.value("id", std::string{});
  if (mapExit.contains("entry") && !mapExit.at("entry").is_null()) {
    exit.targetEntry = mapExit.at("entry").get<std::string>();
  }
  return exit;
}

json loadParty(int slot) {
  const std::string path = slotSavePath(slot);
  if (!std::filesystem::exists(path)) {
    return json::object();
  }
  return openJson(path).at("playerCharacters");
}

std::unique_ptr<Map> mapFromCanonical(const std::string &mapId,
                                      const json &mapJson, const json &party) {
  auto map = std::make_unique<Map>(tmxPathForMap(mapId), 1);
  map->setMapId(mapId);

  for (const auto &[nameId, npc] :
       mapJson.value("characters", json::object()).items()) {
    map->addCharacter(CharacterFactory::create(
        makeCreateJson(nameId, npc.at("type").get<std::string>(),
                       coordFromJson(npc.at("coord")), npc.at("statistics"),
                       npc.value("effectIds", json::array()),
                       npc.value("equipementIds", json::array()))));
  }

  std::optional<Coord> playerCursor;
  bool cursorIsCyrano = false;
  for (const auto &entry : mapJson.value("entries", json::array())) {
    for (const auto &spawn : entry.value("spawn", json::array())) {
      for (const auto &[nameId, info] : spawn.items()) {
        if (!party.contains(nameId)) {
          continue;
        }
        const json &p = party.at(nameId);
        const std::string type = p.at("type").get<std::string>();
        const Coord spawnCoord = coordFromJson(info.at("coord"));

        // Curseur sur Cyrano par défaut
        if (!cursorIsCyrano) {
          if (type == "Cyrano") {
            playerCursor = spawnCoord;
            cursorIsCyrano = true;
          } else if (!playerCursor) {
            playerCursor = spawnCoord;
          }
        }

        map->addCharacter(CharacterFactory::create(makeCreateJson(
            nameId, type, spawnCoord, p.value("statistics", json::object()),
            p.value("effectIds", json::array()),
            p.value("equipementIds", json::array()))));
      }
    }
  }
  if (playerCursor) {
    map->setCursor(*playerCursor);
  }

  for (const auto &exitObj : mapJson.value("exits", json::array())) {
    for (const auto &[name, e] : exitObj.items()) {
      map->addExit(exitFromJson(name, e));
    }
  }

  return map;
}
} // namespace

Map::Map(const std::string &name, int nbLayer) {
  tmxMap.load(name);
  tileSize = tmxMap.getTileSize();
  activeCamera =
      std::make_unique<Camera>(Coord{0, 0}, edgeOffset, viewSize, tmxMap);
  for (int i = 0; i < nbLayer; i++) {
    layers.push_back(std::make_unique<MapLayer>(tmxMap, i));
  }
  computeWalkableGrid();
  tmx::Map cursorTmx;
  cursorTmx.load("resources/cursor/cursor.tmx");
  cursorLayer = std::make_unique<MapLayer>(cursorTmx, 0);

  if (!blueTileTexture.loadFromFile(
          openJson(DATASET).at("blueTile").get<std::string>())) {
    printf("Texture de blueTile non trouvée\n");
  }
  if (!redTileTexture.loadFromFile(
          openJson(DATASET).at("redTile").get<std::string>())) {
    printf("Texture de redTile non trouvée\n");
  }
}

Map::~Map() = default;

void Map::removeDeadCharacters() {
  for (auto it = characters.begin(); it != characters.end();) {
    Character *c = it->get();
    if (c->getStats().life <= 0) {
      if (selectedCharacter == c) {
        selectedCharacter = nullptr;
        walkPath.clear();
        moveRange.clear();
      }
      turnQueue.RemoveCharacter(c);
      it = characters.erase(it);
    } else {
      ++it;
    }
  }
}

Character *Map::FindCharacterByCoord(const Coord &position) {

  Character *playertarget = nullptr;
  for (auto &charac : this->characters) {
    if (charac->getCoord() == position) {
      playertarget = charac.get();
    }
  }

  return playertarget;
}

bool Map::hasPlayerCharacters() const {
  for (const auto &character : characters) {
    if (character->isPlayer()) {
      return true;
    }
  }
  return false;
}

void Map::computeWalkableGrid() {
  const tmx::Vector2u tileCount = tmxMap.getTileCount();
  gridWidth = static_cast<int>(tileCount.x);
  gridHeight = static_cast<int>(tileCount.y);
  walkableGrid.assign(static_cast<int>(gridWidth) * gridHeight, 0);

  for (const auto &layer : tmxMap.getLayers()) {
    if (layer->getType() != tmx::Layer::Type::Tile) {
      continue;
    }
    const auto &tiles = layer->getLayerAs<tmx::TileLayer>().getTiles();
    const int count = std::min(tiles.size(), walkableGrid.size());
    for (int i = 0; i < count; ++i) {
      const std::uint32_t gid = tiles[i].ID;
      if (gid == 0) {
        continue; // empty cell in this layer
      }
      const tmx::Tileset *tileset = tilesetForGid(tmxMap, gid);
      if (tileset == nullptr) {
        continue;
      }
      const tmx::Tileset::Tile *tile = tileset->getTile(gid);
      if (tile == nullptr) {
        continue;
      }
      for (const auto &prop : tile->properties) {
        if (prop.getName() == "walkable" &&
            prop.getType() == tmx::Property::Type::Boolean &&
            prop.getBoolValue()) {
          walkableGrid[i] = 1;
          break;
        }
      }
    }
  }
}

bool Map::isWalkable(Coord coord) const {
  if (coord.x < 0 || coord.y < 0 || coord.x >= gridWidth ||
      coord.y >= gridHeight) {
    return false;
  }
  return walkableGrid[coord.y * gridWidth + coord.x] != 0;
}

void Map::setWalkable(Coord coord, bool walkable) {
  if (coord.x < 0 || coord.y < 0 || coord.x >= gridWidth ||
      coord.y >= gridHeight) {
    return;
  }
  walkableGrid[coord.y * gridWidth + coord.x] = walkable ? 1 : 0;
}

void Map::move() {
  cursorLayer->setOffset(activeCamera->getCursorOffset());
  for (auto &layer : layers) {
    layer->setOffset(activeCamera->getMapOffset());
  }
}

void Map::updateWalkPathAndAV() {
  Coord cursor = this->activeCamera->getCursorCoord();
  if (selectedCharacter == nullptr ||
      selectedCharacter !=
          turnQueue.GetCurrentCharacter()) { // only run if selectedCharacter is
                                             // playable
    return;
  }

  if (!this->attackRange.empty()) {
    turnQueue.UpdateCurrentCharacter(30.0f);
    return;
  }

  if (this->walkPath.empty()) {
    this->walkPath.push_back(selectedCharacter->getCoord());
  }
  Coord previousCase = this->walkPath.back();

  this->walkPath =
      simplePath(this->moveRange, this->selectedCharacter->getCoord(), cursor);

  float case_av = 3.3f; // TO REWORK : No magic number+ take tile + propreties
                        // into account (not implemented yet)
  float total_cost = case_av * (this->walkPath.size());
  turnQueue.UpdateCurrentCharacter(total_cost);
  return;
}

GameState Map::ProcessInputs(std::set<Input> inputs,
                             std::set<Input> justPressedInputs,
                             std::set<Input> inputsRelease,
                             sf::Time deltaTime) {
  Coord cursor = this->activeCamera->getCursorCoord();
  if (getActiveCharacter()->isPlayer()) {
    updateWalkPathAndAV();
    activeCamera->processNewOffset(inputs, inputsRelease, deltaTime);
    this->move();
    if (justPressedInputs.contains(Input::CONFIRM)) {
      if (selectedCharacter == nullptr) {
        selectedCharacter = FindCharacterByCoord(cursor);
        if (selectedCharacter != nullptr) {
          selectedCharacter->setIsCursorSelected(true);
          //              std::cout << selectedCharacter->getNameId();
          this->moveRange = selectedCharacter->calculateMoveRange(
              walkableGrid, gridWidth, gridHeight, characters);
        }
      } else { // selectCharacter != null
        if (cursor == selectedCharacter->getCoord()) {
          this->moveRange.clear();
          this->attackRange =
              selectedCharacter->calculateAttackRange(gridWidth, gridHeight);
        }
        if (selectedCharacter->isPlayer() &&
            selectedCharacter == turnQueue.GetCurrentCharacter()) {
          if (std::ranges::contains(this->moveRange, cursor)) {
            this->moveRange.clear();
            this->moveCharacterTo(selectedCharacter->getNameId(),
                                  this->walkPath, sf::milliseconds(85));
            this->walkPath.clear();
            selectedCharacter->setIsCursorSelected(false);
            selectedCharacter = nullptr;
          }
          if (std::ranges::contains(this->attackRange, cursor) &&
              FindCharacterByCoord(cursor)) {
            this->attackRange.clear();
            this->walkPath.clear();
            selectedCharacter->setIsCursorSelected(false);
            selectedCharacter->lungeAt(*FindCharacterByCoord(cursor));
            selectedCharacter = nullptr;
          }
        }
      }
    }
    if (justPressedInputs.contains(Input::CANCEL)) {
      if (selectedCharacter != nullptr) {
        this->walkPath.clear();
        this->attackRange.clear();
        this->moveRange.clear();
        selectedCharacter->setIsCursorSelected(false);
        selectedCharacter = nullptr;
      }
    }
    if ((justPressedInputs.contains(Input::MENU))) {
      if (selectedCharacter != nullptr) {
        uimanager->LoadCharacterStatsMenu(selectedCharacter); // NOT WORKING T-T
        return GameState::IN_MENU;
      }
    }
  } else {
    auto [action, movePath] = this->getActiveCharacter()->workAI(
        this->walkableGrid, static_cast<int>(tmxMap.getTileCount().x),
        static_cast<int>(tmxMap.getTileCount().y), this->characters);
    if (action == Action::MOVE) {
      moveCharacterTo(getActiveCharacter()->getNameId(), movePath,
                      sf::milliseconds(85));
    }
    if (action == Action::ATTACK) {
      Character *playertarget = FindCharacterByCoord(movePath[0]);
      getActiveCharacter()->lungeAt(*playertarget);
    }
  }

  return DEFAULT_STATE;
}

bool Map::isCinematicActive() const {
  return activeCamera->isCinematicActive();
}

void Map::update(sf::Time elapsed) {
  if (activeCamera->isCinematicActive()) {
    activeCamera->updateCinematic(elapsed);
    for (auto &layer : layers) {
      layer->setOffset(activeCamera->getMapOffset());
    }
  }
  // Attend la fin de la cinematic
  if (pendingMove && !activeCamera->isCinematicActive()) {
    for (auto &character : characters) {
      if (character->getNameId() == pendingMove->nameId) {
        character->moveTo(std::move(pendingMove->path), pendingMove->tileRate);
        break;
      }
    }
    pendingMove.reset();
  }

  for (auto &layer : layers) {
    layer->update(elapsed);
  }
  if (cursorLayer) {
    cursorLayer->update(elapsed);
  }
  for (auto &character : characters) {
    character->update(elapsed);
  }

  removeDeadCharacters();

  if (this->getActiveCharacter()->getTurnEnded()) {
    turnQueue.EndCurrentCharacter();
    saveState(GameApp::getSlot()); // sauvegarde à la fin de chaque tour
  }

  if (selectedCharacter != nullptr) {
    // BLUE
  }

  // Character Map::getCharacter(std::string id) {
  //   for (const auto &character : characters.getObjects()) {
  //     if (character->id == id) {
  //       return character;
  //     }
  //   }
};

void Map::saveState(int slot) const {
  json mapJson;
  mapJson["characters"] = json::object();
  json spawns = json::array();
  json party = json::object();
  for (const auto &character : characters) {
    if (character->isPlayer()) {
      const Coord c = character->getCoord();
      spawns.push_back(
          {{character->getNameId(), {{"coord", {{"x", c.x}, {"y", c.y}}}}}});
      party[character->getNameId()] = character->toPartyJson();
    } else {
      mapJson["characters"][character->getNameId()] = character->toJson();
    }
  }
  mapJson["entries"] = json::array({{{"spawn", spawns}}});

  json exitsJson = json::array();
  for (const auto &exit : exits) {
    json mapExit = {
        {"id", exit.targetMapId},
        {"entry", exit.targetEntry ? json(*exit.targetEntry) : json(nullptr)}};
    exitsJson.push_back({{exit.name,
                          {{"x", exit.coord.x},
                           {"y", exit.coord.y},
                           {"direction", exit.directions},
                           {"mapExit", mapExit}}}});
  }
  mapJson["exits"] = exitsJson;
  writeSave(mapSavePath(slot, mapId), mapJson);

  // Slot save: player party, inventory, current map.
  json slotJson;
  if (std::filesystem::exists(slotSavePath(slot))) {
    slotJson = openJson(slotSavePath(slot));
  }
  slotJson["currentMap"] = mapId;
  if (!slotJson.contains("inventory")) {
    slotJson["inventory"] = json::array();
  }
  slotJson["playerCharacters"] = party;
  writeSave(slotSavePath(slot), slotJson);
}

std::unique_ptr<Map> Map::loadMap(UIManager *uim, int slot,
                                  const std::string &mapId) {
  CharacterFactory::registerBuiltins();

  const std::string mapPath = mapSavePath(slot, mapId);
  const bool fromSave = std::filesystem::exists(mapPath);
  const json mapJson = openJson(fromSave ? mapPath : defaultMapPath(mapId));

  // First load of the slot (no save yet): seed the party from the new-game
  // template; otherwise the slot save is the source of truth.
  json party;
  if (std::filesystem::exists(slotSavePath(slot))) {
    party = loadParty(slot);
  } else {
    party = openJson(DEFAULT_SAVE).value("playerCharacters", json::object());
  }

  auto map = mapFromCanonical(mapId, mapJson, party);
  if (!fromSave) {
    map->saveState(slot);
  }

  std::vector<std::pair<Character *, float>> queue = {};
  if (false &&
      std::filesystem::exists(slotSavePath(slot))) { // TBA : queue save
    //    queue = loadQueue(slot);
  } else {
    for (const auto &characPtr : map->characters) {
      assert(characPtr->getStats().speed > 0);
      queue.push_back(
          {characPtr.get(),
           (BASE_DEFAULT_AV / (float)characPtr->getStats()
                                  .speed)}); // PB HERE : Speed set to 0 ????
    }
  }

  assert(queue.size() == map->characters.size());

  map->turnQueue.SetQueue(queue);
  return map;
}

void Map::drawOverTiles(const sf::Texture &tile_tex, sf::RenderTarget &target,
                        sf::RenderStates states,
                        const std::vector<Coord> &tiles) const {
  sf::RenderStates tileStates = states;
  tileStates.transform.translate(activeCamera->getMapOffset());

  for (const auto &coord : tiles) {
    sf::Sprite sprite(tile_tex);
    // 50% de transparence (alpha 128 sur 255).
    sprite.setColor(sf::Color(255, 255, 255, 128));
    sprite.setPosition(
        {static_cast<float>(coord.x) * static_cast<float>(tileSize.x),
         static_cast<float>(coord.y) * static_cast<float>(tileSize.y)});
    target.draw(sprite, tileStates);
  }
}
