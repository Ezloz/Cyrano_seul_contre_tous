#include "Save.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

json openJson(const std::string &file) {
  std::ifstream f(file);
  if (!f)
    throw std::runtime_error(std::string("Cannot open json file: ") + file);
  return json::parse(f);
}

// écriture atomique de la save (rename)
void writeSave(const std::string &savePath, const json &content) {
  const std::filesystem::path parent = std::filesystem::path(savePath).parent_path();
  if (!parent.empty()) {
    std::filesystem::create_directories(parent);
  }
  std::string tmpPath = savePath + ".tmp";
  {
    std::ofstream o(tmpPath);
    if (!o)
      throw std::runtime_error("Cannot open temporary file: " + tmpPath);
    o << content;
  }
  std::filesystem::rename(tmpPath, savePath);
}

std::string slotSavePath(int slot) {
  return "saves/slot" + std::to_string(slot) + "/save.json";
}

std::string mapSavePath(int slot, const std::string &mapId) {
  return "saves/slot" + std::to_string(slot) + "/Maps/" + mapId +
         "/mapSave.json";
}

std::string defaultMapPath(const std::string &mapId) {
  return "resources/Maps/" + mapId + "/defaultMap.json";
}
