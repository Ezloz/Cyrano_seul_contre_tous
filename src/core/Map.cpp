#include "Map.h"

#include "CharacterFactory.h"
#include "Save.h"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Property.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Tileset.hpp>

#include <algorithm>
#include <filesystem>

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

json seedParty(const json &mapJson) {
  json party = json::object();
  const json zeroStats = {{"life", 0},  {"speed", 0}, {"charisma", 0},
                          {"power", 0}, {"luck", 0},  {"range", 0}};
  for (const auto &entry : mapJson.value("entries", json::array())) {
    for (const auto &spawn : entry.value("spawn", json::array())) {
      for (const auto &[nameId, info] : spawn.items()) {
        if (!CharacterFactory::isRegistered(nameId)) {
          continue;
        }
        party[nameId] = {{"type", nameId},
                         {"statistics", zeroStats},
                         {"effectIds", json::array()},
                         {"equipementIds", json::array()}};
      }
    }
  }
  return party;
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
  // The cursor uses its own tmx::Map so tmxMap keeps holding the game map.
  cursorTmx.load("resources/cursor/cursor.tmx");
  cursorLayer = std::make_unique<MapLayer>(cursorTmx, 0);
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

void Map::move(std::set<Input> inputs, std::set<Input> inputsRelease,
               sf::Time deltaTime) {
  activeCamera->processNewOffset(inputs, inputsRelease, deltaTime);
  cursorLayer->setOffset(activeCamera->getCursorOffset());
  for (auto &layer : layers) {
    layer->setOffset(activeCamera->getMapOffset());
  }
}

void Map::startCinematic(Coord from, Coord to, sf::Time duration) {
  activeCamera->generateCinematic(from, to, duration);
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

  for (auto &layer : layers) {
    layer->update(elapsed);
  }
  if (cursorLayer) {
    cursorLayer->update(elapsed);
  }
  for (auto &character : characters) {
    character->update(elapsed);
  }

  // Character Map::getCharacter(std::string id) {
  //   for (const auto &character : characters.getObjects()) {
  //     if (character->id == id) {
  //       return character;
  //     }
  //   }
};

void Map::saveState(int slot) const {
  // Map state (canonical shape): NPCs keyed by nameId, player spawn positions,
  // and exits. Player identity/stats go to the slot save, not here.
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

  json party;
  if (std::filesystem::exists(slotSavePath(slot))) {
    party = loadParty(slot);
  } else {
    party = seedParty(openJson(defaultMapPath(mapId)));
  }

  auto map = mapFromCanonical(mapId, mapJson, party);
  if (!fromSave) {
    map->saveState(slot);
  }

  std::vector<std::pair<Character*, float>> queue = {};
  for (const auto& characPtr : map->characters){
    std::cout << characPtr->getStats().speed;
    queue.push_back({characPtr.get(),(BASE_DEFAULT_AV/(float) characPtr->getStats().speed)}); //PB HERE : Speed set to 0 ????
  }
  map->turnQueue.SetQueue(queue);
  map->activeCharacter = map->turnQueue.GetCurrentCharacter();
  return map;
}
