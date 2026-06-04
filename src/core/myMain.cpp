#include "myMain.h"

#include <iostream>

#include <SFML/Graphics.hpp>


using namespace std;
int myMain() {

  sf::RenderWindow window{sf::VideoMode({800, 800}), "SFML works!"};
  window.setFramerateLimit(30);

  sf::RectangleShape white_rectangle{sf::Vector2f(800,800)};
  white_rectangle.setFillColor(sf::Color(255,255,255));
  
  while (window.isOpen()) {
    // check all the window's events that were triggered since the last
    // iteration of the loop
    while (const std::optional event = window.pollEvent()) {
      // "close requested" event: we close the window
      if (event->is<sf::Event::Closed>()) window.close();
      
      // Catch the resize events
      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        // Update the view to the new size of the window, so that contents is
        // not distorted by resizing See
        // https://www.sfml-dev.org/tutorials/3.0/graphics/view/#using-a-view
        sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
        window.setView(sf::View(visibleArea));
      }
    }
    window.clear();
    
    window.draw(white_rectangle);
    window.display();
  }



  return 0;
}