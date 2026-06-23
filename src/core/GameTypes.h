#pragma once

// A tile position on the map grid.
struct Coord {
  int x = 0;
  int y = 0;
};

static bool operator==(const Coord& a, const Coord& b) {return a.x == b.x && a.y == b.y;}


enum class Input { UP, DOWN, LEFT, RIGHT, CONFIRM, CANCEL, MENU, NB_INPUTS };

enum class GameState {
  IN_GAME,
  IN_GAMEMENU,
  IN_MENU,
  IN_BATTLE_ANIMATION,
  IN_CINEMATIC
};
