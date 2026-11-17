#include <SFML/Graphics.hpp>

#include <optional>

int main()
{
    sf::RenderWindow window(sf::VideoMode({640, 400}), "Circle Sample");
    window.setFramerateLimit(60);
    sf::CircleShape circle(75.f);
    circle.setFillColor(sf::Color(56, 189, 248));
    circle.setPosition({245.f, 125.f});

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        window.clear(sf::Color(16, 22, 38));
        window.draw(circle);
        window.display();
    }
}
