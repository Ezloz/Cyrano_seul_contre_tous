#include "myMain.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "GameApp.h"
#include "Map.h"

int myMain() {

  sf::RenderWindow window{sf::VideoMode({800, 600}), "SFML works!"};
  window.setFramerateLimit(30);

  GameApp *gameInstance = GameApp::GetInstance();
  Map map("resources/little_test_20.tmx", 1);

  while (window.isOpen()) {
    // Restart the frame clock and clear the previous frame's released inputs
    // before polling, so this frame's releases survive until map.move().
    gameInstance->Update();

    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        gameInstance->Quit();
        window.close();
      }

      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
        window.setView(sf::View(visibleArea));
      } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        gameInstance->transformRawInputToInput(keyPressed);
      } else if (const auto *keyUnpressed = event->getIf<sf::Event::KeyReleased>()) {
        gameInstance->transformRawInputToInput(keyUnpressed);
      }
    }

    gameInstance->ProcessInputs();
    map.move(gameInstance->GetInputs(), gameInstance->GetReleasedInputs());

    window.clear(sf::Color::Black);
    window.draw(map);
    window.display();
  }

  return 0;
}
