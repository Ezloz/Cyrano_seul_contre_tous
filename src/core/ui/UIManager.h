#pragma once
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include "app/GameTypes.h"
#include "entities/Character.h"

class UIManager {
  private:
  tgui::Gui gui;
  std::string current_name = "None";
  
  void addRow(const tgui::String& name);
  
  public:
  explicit UIManager(sf::RenderWindow& window) : gui{window} { /*LoadOptions();*/ };

  void Unload();

  void LoadCharacterStatsMenu(const Character* character);
  
  bool LoadGUI(const std::string& pathname);

  GameState ProcessInputs(std::set<Input> inputs, std::set<Input> justPressedInputs, std::set<Input> inputsRelease, sf::Time deltaTime);

  GameState move(std::set<Input> inputs, std::set<Input> justPressedInputs, std::set<Input> inputsRelease, sf::Time deltaTime);

  void draw(){gui.draw();};

  void update(sf::RenderWindow& window) {}
};