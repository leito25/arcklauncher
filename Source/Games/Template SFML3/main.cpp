#include <SFML/Graphics.hpp>
#include "ArcadeSDK.h"

#include <chrono>
#include <iostream>
#include <optional>

int main()
{
    if (ArcadeInit("template-sfml3", nullptr) != ARCADE_OK) {
        std::cerr << "Arcade SDK initialization failed: " << ArcadeLastError() << '\n';
        return 1;
    }
    const auto started = std::chrono::steady_clock::now();
    sf::RenderWindow window(sf::VideoMode({640, 400}), "Circle Sample (SFML 3)");
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
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - started).count();
    if (ArcadeAddPlaytime(static_cast<uint64_t>(seconds)) != ARCADE_OK) {
        std::cerr << "Could not save playtime: " << ArcadeLastError() << '\n';
        ArcadeShutdown();
        return 1;
    }
    if (ArcadeShutdown() != ARCADE_OK) {
        std::cerr << "Arcade SDK shutdown failed: " << ArcadeLastError() << '\n';
        return 1;
    }
}
