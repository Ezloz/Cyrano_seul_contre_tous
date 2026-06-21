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

  GameApp gameInstance{&window};
  // A remplacer lors du choix du joueur (slot) et de la map de départ.
  const int slot = 1;
  const std::string startMap = "castleBlue";
  std::unique_ptr<Map> map = Map::loadMap(slot, startMap);

  const tmx::Vector2u tileSize = map->GetTileSize();
  const Coord viewTiles = map->GetViewSize();
  const sf::Vector2f viewSize(viewTiles.x * tileSize.x,
                              viewTiles.y * tileSize.y);
  sf::View view(viewSize / 2.f, viewSize);
  window.setView(view);

  gameInstance.Start();

  const Coord originCorner = {0, 0};
  const Coord introCorner = {5, 3};
  const sf::Time cinematicDuration = sf::seconds(1.f);
  int introPhase = 0;
  map->startCinematic(originCorner, introCorner, cinematicDuration);

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

        window.setView(view);
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
    if (introPhase >= 2) {
      map->move(gameInstance.GetInputs(), gameInstance.GetReleasedInputs(),
                gameInstance.GetDeltaTime());
    }
    map->update(gameInstance.GetDeltaTime());

    if (introPhase < 2 && !map->isCinematicActive()) {
      if (introPhase == 0) {
        map->startCinematic(introCorner, originCorner, cinematicDuration);
        introPhase = 1;
      } else {
        introPhase = 2;
      }
    }

    if (introPhase >= 2 && !moveTestStarted) {
      map->moveCharacterTo("Cyrano", {{3, 3}, {6, 3}, {6, 5}},
                           sf::seconds(0.5f));
      moveTestStarted = true;
    }
    //END TO SUPPRESS/MOVE
    
    window.clear(sf::Color::Black);
    window.draw(*map); // TO SUPRESS/MOVE
    gameInstance.Draw(window);
    window.display();
  }

  return 0;
}
