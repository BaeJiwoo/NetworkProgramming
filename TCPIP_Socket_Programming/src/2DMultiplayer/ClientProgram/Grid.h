#pragma once
#include <SFML/Graphics.hpp>

class Grid
{
public:
    Grid(sf::Vector2f worldSize, float cellSize);
    void Draw(sf::RenderWindow& window) const;

private:
    sf::VertexArray m_lines;
};