#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

class UIManager {
  private:
  tgui::Gui gui;
  
  
  public:
  explicit UIManager(sf::RenderWindow *window) : gui{*window} { /*LoadOptions();*/ };
  
  bool LoadGUI(std::string pathname);

  void draw(sf::RenderWindow* window = NULL){gui.draw();};
};