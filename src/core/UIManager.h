#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

#include "GameTypes.h"

class UIManager {
  private:
  tgui::Gui gui;
  std::string current_name;
  
  
  public:
  explicit UIManager(sf::RenderWindow& window) : gui{window} { /*LoadOptions();*/ };
  
  bool LoadGUI(std::string pathname);

  GameState ProcessInputs(GameState state, std::set<Input> inputs, std::set<Input> inputsRelease, sf::Time deltaTime){
    if (state == GameState::IN_MENU){
      state = this->move(inputs, inputsRelease, deltaTime);
    }
    return state;
  }

  GameState move(std::set<Input> inputs, std::set<Input> inputsRelease, sf::Time deltaTime);

  void draw(sf::RenderWindow* window = NULL){gui.draw();};
};