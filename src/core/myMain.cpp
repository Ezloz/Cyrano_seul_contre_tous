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
  Map map("resources/little_test_20.tmx", 1);

  const tmx::Vector2u tileSize = map.GetTileSize();
  const Coord viewTiles = map.GetViewSize();
  const sf::Vector2f viewSize(viewTiles.x * tileSize.x,
                              viewTiles.y * tileSize.y);
  sf::View view(viewSize / 2.f, viewSize);
  window.setView(view);

  gameInstance.LoadGUI(" ");

  // Intro: pan the view out to a corner, then back to the original offset,
  // and only then reveal the cursor and hand control to the player.
  const Coord originCorner = {0, 0};
  const Coord introCorner = {5, 3};
  const sf::Time cinematicDuration = sf::seconds(1.f);
  // 0: panning out, 1: panning back, 2: gameplay.
  int introPhase = 0;
  map.startCinematic(originCorner, introCorner, cinematicDuration);

  while (window.isOpen()) {
    gameInstance.Update();

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
    // Ignore player movement until the intro cinematics are done.
    if (introPhase >= 2) {
      map.move(gameInstance.GetInputs(), gameInstance.GetReleasedInputs(),
               gameInstance.GetDeltaTime());
    }
    map.update(gameInstance.GetDeltaTime());

    // Chain the intro legs: when one finishes, start the next one (or end the
    // intro) before drawing, so the cursor never flashes between legs.
    if (introPhase < 2 && !map.isCinematicActive()) {
      if (introPhase == 0) {
        map.startCinematic(introCorner, originCorner, cinematicDuration);
        introPhase = 1;
      } else {
        introPhase = 2;
      }
    }

    window.clear(sf::Color::Black);
    window.draw(map);
    gameInstance.Draw(window);
    window.display();
  }

  return 0;
}
