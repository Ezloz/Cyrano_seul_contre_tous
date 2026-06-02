#include "myMain.h"

#include <iostream>

#include "Circle.h"
#include "Group.h"
#include "Drawing.h"
#include "pugixml.hpp"
#include <SFML/Graphics.hpp>


void modify_drawing(Drawing &d) {
  d.find_by_label("nez")->set_color("Yellow"); // Si set_color est appliqué à un groupe, il doit être 
                                                     // appliqué à chaque élément du groupe.
  d.find_by_label("c22")->translate(-10,-10);
  // d.find_by_label("oreille1")->clone();  // Le groupe "oreille1" est cloné en un groupe de label
  //                                              // "oreille1+" contenant les cercles "ci1+" et "ci2+",
  //                                              // clones de "ci1" et "ci2".
  std::cout << d.dump("") << std::endl;
  // d.find_by_label("oreille1+")->set_color("Blue");
  // d.find_by_label("oreille1+")->translate(0, -455);
  // d.find_by_label("c12+")->translate(210, -30);
  // d.find_by_label("c11+")->set_color("Cyan");
  // d.find_by_label("c11+")->clone(); // Le cercle "c11+" est cloné en cercle "c11++".
  // d.find_by_label("c11++")->translate(420, 0);
}

using namespace std;
int myMain() {

  pugi::xml_document doc;
  if (auto result = doc.load_file("resources/visage.xml"); !result) {
    std::cerr << "Could not open file visage.xml because " << result.description() << std::endl;
    return 1;
  }

  pugi::xml_node node = doc.child("Drawing");

//  cout << "main : " << node.first_child().name() << endl;

  Drawing d{node};
  
  modify_drawing(d);

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
    d.draw(window, 400, 400);
    window.display();
  }



  return 0;
}