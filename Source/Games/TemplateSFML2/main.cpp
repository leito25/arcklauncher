#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(640, 400), "Circle Sample (SFML 2)");
    window.setFramerateLimit(60);
    sf::CircleShape circle(75.f);
    circle.setFillColor(sf::Color(251, 146, 60));
    circle.setPosition(245.f, 125.f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color(16, 22, 38));
        window.draw(circle);
        window.display();
    }
}
