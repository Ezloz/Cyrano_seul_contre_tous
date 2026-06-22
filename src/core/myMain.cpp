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

  GameApp gameInstance{window};
  gameInstance.Start();
  // A remplacer lors du choix du joueur (slot) et de la map de départ.

  window.setView(gameInstance.GetViewSize());


  const Coord originCorner = {0, 0};
  const Coord introCorner = {5, 3};
  const sf::Time cinematicDuration = sf::seconds(1.f);
  int introPhase = 0;
//  gameInstance->activeMap->startCinematic(originCorner, introCorner, cinematicDuration);

  bool moveTestStarted = false;

  while (window.isOpen()) {

    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        gameInstance.Quit();
        window.close();
      }

      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        // sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
        // window.setView(sf::View(visibleArea));

        window.setView(gameInstance.GetViewSize());
      } else if (const auto *keyPressed =
                     event->getIf<sf::Event::KeyPressed>()) {
        gameInstance.transformRawInputToInput(keyPressed);
      } else if (const auto *keyUnpressed =
                     event->getIf<sf::Event::KeyReleased>()) {
        gameInstance.transformRawInputToInput(keyUnpressed);
      }
    }

    gameInstance.ProcessInputs();
    gameInstance.Update();


    //TO SUPPRESS/MOVE
  /*
  if (introPhase >= 2) {
    gameInstance->activeMap->move(gameInstance.GetInputs(), gameInstance.GetReleasedInputs(),
    gameInstance.GetDeltaTime());
  }
  gameInstance->activeMap->update(gameInstance.GetDeltaTime());
  
  if (introPhase < 2 && !gameInstance->activeMap->isCinematicActive()) {
    if (introPhase == 0) {
      gameInstance->activeMap->startCinematic(introCorner, originCorner, cinematicDuration);
      introPhase = 1;
    } else {
      introPhase = 2;
    }
  }
  
  if (introPhase >= 2 && !moveTestStarted) {
    gameInstance->activeMap->moveCharacterTo("Cyrano", {{3, 3}, {6, 3}, {6, 5}},
    sf::seconds(0.5f));
    moveTestStarted = true;
  }
*/
    //END TO SUPPRESS/MOVE
    
    window.clear(sf::Color::Black);
    gameInstance.Draw(window);
    window.display();
  }

  return 0;
}
