#pragma once
#include <SFML/Graphics.hpp>

class Player
{
public:
    Player();

    void Update(float dt, const sf::Vector2f& worldSize);
    void Draw(sf::RenderWindow& window);
    sf::Vector2f GetPosition() const;

private:
    sf::Vector2f Normalize(sf::Vector2f v);

private:
    sf::RectangleShape shape;
    float speed;
};