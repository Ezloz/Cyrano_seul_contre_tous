#include "app/myMain.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "app/GameApp.h"
#include "world/Map.h"

int myMain() {

  sf::RenderWindow window{sf::VideoMode({800, 600}), "Cyrano - Seul contre tous"};
  window.setFramerateLimit(30);

  GameApp gameInstance{window};
  gameInstance.Start();

  window.setView(gameInstance.GetViewSize());



  while (window.isOpen()) {

    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        gameInstance.SaveGame();
        gameInstance.Quit();
        window.close();
      }

//      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        // sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
        // window.setView(sf::View(visibleArea));
//        window.setView(gameInstance.GetViewSize());
//      } 
      if (const auto *keyPressed =
                     event->getIf<sf::Event::KeyPressed>()) {
        gameInstance.transformRawInputToInput(keyPressed);
      } else if (const auto *keyUnpressed =
                     event->getIf<sf::Event::KeyReleased>()) {
        gameInstance.transformRawInputToInput(keyUnpressed);
      }
    }

    gameInstance.ProcessInputs();
    gameInstance.Update();

    
    window.clear(sf::Color::Black);
    gameInstance.Draw(window);
    window.display();
  }

  return 0;
}


