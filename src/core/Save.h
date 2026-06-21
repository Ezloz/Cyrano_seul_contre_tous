#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

inline constexpr const char *DATASET = "resources/dataset.json";

json openJson(const std::string &file);
void writeSave(const std::string &savePath, const json &content);
std::string slotSavePath(int slot);

// Utiliser lors du changement de map
std::string mapSavePath(int slot, const std::string &mapId);

// Toute premiere map chargée
std::string defaultMapPath(const std::string &mapId);
