#pragma once

#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Grid.h"

class Game
{
public:
    Game();
    void Run();

private:
    void ProcessEvents();
    void Update(float dt);
    void Render();

private:
    sf::RenderWindow window;
    sf::View camera;

    Player player;
    Grid grid;

    sf::Vector2f worldSize;
};