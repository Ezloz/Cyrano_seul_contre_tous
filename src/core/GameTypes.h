#pragma once
#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

inline constexpr const char *DATASET = "resources/dataset.json";

inline json openJson(std::string file) {
  std::ifstream f(file);
  if (!f)
    throw std::runtime_error(std::string("Cannot open json file: ") + file);
  return json::parse(f);
}

// A tile position on the map grid.
struct Coord {
  int x = 0;
  int y = 0;
};

enum class Input { UP, DOWN, LEFT, RIGHT, CONFIRM, CANCEL, MENU, NB_INPUTS };

enum class GameState {
  IN_GAME,
  IN_GAMEMENU,
  IN_MENU,
  IN_ANIMATION,
  IN_CINEMATIC
};
