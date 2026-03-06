#include "Game.h"
#include <optional>

Game::Game()
    : window(sf::VideoMode({ 800, 600 }), "SFML Game")
    , camera(sf::Vector2f(400.f, 300.f), sf::Vector2f(800.f, 600.f))
    , grid(sf::Vector2f(2000.f, 1200.f), 64.f)
{
    worldSize = sf::Vector2f(2000.f, 1200.f);
    window.setFramerateLimit(60);
}

void Game::Run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        ProcessEvents();
        Update(dt);
        Render();
    }
}

void Game::ProcessEvents()
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
    }
}

void Game::Update(float dt)
{
    player.Update(dt, worldSize);

    camera.setCenter(player.GetPosition());
    window.setView(camera);
}

void Game::Render()
{
    window.clear();

    grid.Draw(window);
    player.Draw(window);

    window.display();
}