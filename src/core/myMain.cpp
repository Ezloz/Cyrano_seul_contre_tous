#include "myMain.h"

#include <iostream>

 #include <SFML/Graphics.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "MapLayer.h"
#include "GameApp.h"

using namespace std;
int myMain() {

  sf::RenderWindow window{sf::VideoMode({800, 600}), "SFML works!"};
  tmx::Map map;
  map.load("resources/little_test_20.tmx");
  //
  MapLayer layerZero(map, 0);
  // MapLayer layerOne(map, 1);
  // MapLayer layerTwo(map, 2);


  sf::Clock wiggleClock;
  bool doWiggle = false;
  
  window.setFramerateLimit(30);
  
//  sf::RectangleShape white_rectangle{sf::Vector2f(800,800)};
//  white_rectangle.setFillColor(sf::Color(255,255,255));
  GameApp* gameInstance = GameApp::GetInstance();

  while (window.isOpen()) {
    // check all the window's events that were triggered since the last
    // iteration of the loop
    while (const std::optional event = window.pollEvent()) {
      // "close requested" event: we close the window
        if (event->is<sf::Event::Closed>())
        {
          gameInstance->Quit();
          window.close();
        }
      
      // Catch the resize events
      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        // Update the view to the new size of the window, so that contents is
        // not distorted by resizing See
        // https://www.sfml-dev.org/tutorials/3.0/graphics/view/#using-a-view
        sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
        window.setView(sf::View(visibleArea));
      }

      else if(const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
      {
        gameInstance->HandleRawInput(keyPressed);
      }
      else if(const auto* keyUnpressed = event->getIf<sf::Event::KeyReleased>())
      {
        gameInstance->HandleRawInput(keyUnpressed);
      }
    }

    gameInstance->ProcessInputs();
    gameInstance->Update();
    // layerZero.update(duration);

    sf::Vector2f newOffset = sf::Vector2f(0.f, 0.f);
    if (doWiggle)
    {
          newOffset = sf::Vector2f(std::cos(wiggleClock.getElapsedTime().asSeconds()) * 100.f, 0.f);
    }
    layerZero.setOffset(newOffset);
    // layerOne.setOffset(newOffset);
    // layerTwo.setOffset(newOffset);

    window.clear(sf::Color::Black);
    window.draw(layerZero);
    // window.draw(layerOne);
    // window.draw(layerTwo);
    window.display();
  }



  return 0;
}
