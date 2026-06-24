#include "world/Map.h"

#include "entities/CharacterFactory.h"
#include "io/Save.h"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Property.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Tileset.hpp>

#include <ranges>
#include <algorithm>
#include <filesystem>
#include <queue>
#include <unordered_set>

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

  for (const auto &entry : mapJson.value("entries", json::array())) {
    for (const auto &spawn : entry.value("spawn", json::array())) {
      for (const auto &[nameId, info] : spawn.items()) {
        if (!party.contains(nameId)) {
          continue;
        }
        const json &p = party.at(nameId);
        map->addCharacter(CharacterFactory::create(
            makeCreateJson(nameId, p.at("type").get<std::string>(),
                           coordFromJson(info.at("coord")),
                           p.value("statistics", json::object()),
                           p.value("effectIds", json::array()),
                           p.value("equipementIds", json::array()))));
      }
    }
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
}

Map::~Map() = default;

void Map::computeWalkableGrid() {
  const tmx::Vector2u tileCount = tmxMap.getTileCount();
  gridWidth = static_cast<int>(tileCount.x);
  gridHeight = static_cast<int>(tileCount.y);
  walkableGrid.assign(static_cast<std::size_t>(gridWidth) * gridHeight, 0);

  for (const auto &layer : tmxMap.getLayers()) {
    if (layer->getType() != tmx::Layer::Type::Tile) {
      continue;
    }
    const auto &tiles = layer->getLayerAs<tmx::TileLayer>().getTiles();
    const std::size_t count = std::min(tiles.size(), walkableGrid.size());
    for (std::size_t i = 0; i < count; ++i) {
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
  return walkableGrid[static_cast<std::size_t>(coord.y) * gridWidth +
                      coord.x] != 0;
}

void Map::setWalkable(Coord coord, bool walkable) {
  if (coord.x < 0 || coord.y < 0 || coord.x >= gridWidth ||
      coord.y >= gridHeight) {
    return;
  }
  walkableGrid[static_cast<std::size_t>(coord.y) * gridWidth + coord.x] =
      walkable ? 1 : 0;
}

void Map::move() {
  cursorLayer->setOffset(activeCamera->getCursorOffset());
  for (auto &layer : layers) {
    layer->setOffset(activeCamera->getMapOffset());
  }
}

struct CoordHash {
    size_t operator()(const Coord& c) const {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1);
    }
};

std::vector<Coord> BFS(
    const std::vector<Coord>& moveRange,
    const Coord& start,
    const Coord& target)
{
    std::unordered_set<Coord, CoordHash> walkable(moveRange.begin(), moveRange.end());

    std::queue<Coord> q;
    std::unordered_map<Coord, Coord, CoordHash> parent;
    std::unordered_set<Coord, CoordHash> visited;

    auto push = [&](const Coord& from, const Coord& to) {
        if (walkable.count(to) && !visited.count(to)) {
            visited.insert(to);
            parent[to] = from;
            q.push(to);
        }
    };

    q.push(start);
    visited.insert(start);

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    bool found = false;

    while (!q.empty() && !found) {
        Coord cur = q.front();
        q.pop();

        for (int i = 0; i < 4; i++) {
            Coord nxt{cur.x + dx[i], cur.y + dy[i]};

            if (nxt == target) {
                parent[nxt] = cur;
                found = true;
                break;
            }

            push(cur, nxt);
        }
    }

    // Reconstruct path
    std::vector<Coord> path;
    Coord cur = target;
    path.push_back(cur);

    while (!(cur == start)) {
        cur = parent[cur];
        path.push_back(cur);
    }

    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<Coord> simplePath(const std::vector<Coord>& moveRange, const Coord& start, const Coord& target)
{
  auto it = std::ranges::min_element(moveRange,
      [&](const Coord& a, const Coord& b) {
          return manhattanDistance(a, target) < manhattanDistance(b, target);
      });

  Coord closestCoord = *it;

  return BFS(moveRange, start, target);

}


void Map::updateWalkPathAndAV() {
  Coord cursor = this->activeCamera->getCursorCoord();
  if (selectedCharacter == nullptr ||
      selectedCharacter !=
          turnQueue.GetCurrentCharacter()) { // only run if selectedCharacter is
                                             // playable
    return;
  }
  if (this->walkPath.empty()) {
    this->walkPath.push_back(selectedCharacter->getCoord());
  }
  Coord previousCase = this->walkPath.back();
  if (cursor == previousCase) {
    return;
  }
  if (isInRange(selectedCharacter->getCoord(), cursor, selectedCharacter->getStats().range)) {
    if (isInRange(previousCase, cursor, 1) &&
        this->walkPath.size() <= selectedCharacter->getStats().range) {
      this->walkPath.erase(std::find(this->walkPath.begin(), this->walkPath.end(), cursor), this->walkPath.end());
      this->walkPath.push_back(cursor);
      std::cout << cursor.x << "," << cursor.y;
    } else {
      this->walkPath = simplePath(this->moveRange, this->selectedCharacter->getCoord(), cursor);
    }
    float case_av = 100.0f / selectedCharacter->getStats().speed; // TO REWORK : No magic number+ take tile + propreties into account (not implemented yet)
    turnQueue.UpdateCurrentCharacter(case_av * (this->walkPath.size() - 1));
    return;
  }
}

GameState Map::ProcessInputs(GameState state, std::set<Input> inputs,
                             std::set<Input> inputsRelease,
                             sf::Time deltaTime) {
  if (state == GameState::IN_GAME) {
    Coord cursor = this->activeCamera->getCursorCoord();
    updateWalkPathAndAV();
    if (getActiveCharacter()->isPlayer()) {
      activeCamera->processNewOffset(inputs, inputsRelease, deltaTime);
      this->move();
      if (inputs.find(Input::CONFIRM) !=
          inputs.end()) { // To REWORK : Single press
        if (selectedCharacter == nullptr) {
          for (const auto &charac : this->characters) {
            if (charac->getCoord() == cursor) {
              selectedCharacter = charac.get();
              //              std::cout << selectedCharacter->getNameId();
              this->moveRange = selectedCharacter->calculateMoveRange(
                  walkableGrid, gridWidth, gridHeight, characters);
            }
          }
        } else {
          if (selectedCharacter->isPlayer() &&
              selectedCharacter == turnQueue.GetCurrentCharacter()

              && isInRange(selectedCharacter->getCoord(), cursor,
                           selectedCharacter->getStats().range) &&
              !(cursor == selectedCharacter->getCoord())) {
            this->moveRange.clear();

            this->moveCharacterTo(selectedCharacter->getNameId(),
                                  this->walkPath, sf::milliseconds(85));

            this->walkPath.clear();
            selectedCharacter = nullptr;

            turnQueue.EndCurrentCharacter();
          }
        }
      }
      if (inputs.find(Input::CANCEL) != inputs.end()) {
        turnQueue.UpdateCurrentCharacter(0);
        this->walkPath.clear();
        this->moveRange.clear();
        selectedCharacter = nullptr;
      }
    } else {
      if (this->getActiveCharacter()->workAI(
              this->walkableGrid, static_cast<int>(tmxMap.getTileCount().x),
              static_cast<int>(tmxMap.getTileCount().y), this->characters)) {
        turnQueue.EndCurrentCharacter();
      }
    }
  }

  return state;
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

std::unique_ptr<Map> Map::loadMap(int slot, const std::string &mapId) {
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
  for (const auto &characPtr : map->characters) {
    std::cout << characPtr->getStats().speed;
    //    queue.push_back({characPtr.get(),(BASE_DEFAULT_AV/(float)
    //    characPtr->getStats().speed)}); //PB HERE : Speed set to 0 ????
    queue.push_back(
        {characPtr.get(),
         (BASE_DEFAULT_AV / 10.0f)}); // PB HERE : Speed set to 0 ????
  }
  map->turnQueue.SetQueue(queue);
  return map;
}

void Map::drawBlueTiles(sf::RenderTarget &target, sf::RenderStates states,
                        const std::vector<Coord> &blueTiles) const {
  sf::RenderStates tileStates = states;
  tileStates.transform.translate(activeCamera->getMapOffset());

  for (const auto &coord : blueTiles) {
    sf::Sprite sprite(blueTileTexture);
    // 50% de transparence (alpha 128 sur 255).
    sprite.setColor(sf::Color(255, 255, 255, 128));
    sprite.setPosition(
        {static_cast<float>(coord.x) * static_cast<float>(tileSize.x),
         static_cast<float>(coord.y) * static_cast<float>(tileSize.y)});
    target.draw(sprite, tileStates);
  }
}
