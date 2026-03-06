#include "Grid.h"

Grid::Grid(sf::Vector2f worldSize, float cellSize)
    : m_lines(sf::PrimitiveType::Lines)
{
    for (float x = 0.f; x <= worldSize.x; x += cellSize)
    {
        sf::Vertex v1;
        v1.position = sf::Vector2f(x, 0.f);
        v1.color = sf::Color(80, 80, 80);
        m_lines.append(v1);

        sf::Vertex v2;
        v2.position = sf::Vector2f(x, worldSize.y);
        v2.color = sf::Color(80, 80, 80);
        m_lines.append(v2);
    }

    for (float y = 0.f; y <= worldSize.y; y += cellSize)
    {
        sf::Vertex v1;
        v1.position = sf::Vector2f(0.f, y);
        v1.color = sf::Color(80, 80, 80);
        m_lines.append(v1);

        sf::Vertex v2;
        v2.position = sf::Vector2f(worldSize.x, y);
        v2.color = sf::Color(80, 80, 80);
        m_lines.append(v2);
    }
}

void Grid::Draw(sf::RenderWindow& window) const
{
    window.draw(m_lines);
}