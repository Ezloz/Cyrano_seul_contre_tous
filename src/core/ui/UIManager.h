#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include "app/GameTypes.h"

class UIManager {
  private:
  tgui::Gui gui;
  std::string current_name;
  
  
  public:
  explicit UIManager(sf::RenderWindow& window) : gui{window} { /*LoadOptions();*/ };
  
  bool LoadGUI(const std::string& pathname);

  GameState ProcessInputs(std::set<Input> inputs, std::set<Input> pressedInputs, std::set<Input> inputsRelease, sf::Time deltaTime);

  GameState move(std::set<Input> inputs, std::set<Input> inputsRelease, sf::Time deltaTime);

  void draw(sf::RenderWindow* window = NULL){gui.draw();};
};