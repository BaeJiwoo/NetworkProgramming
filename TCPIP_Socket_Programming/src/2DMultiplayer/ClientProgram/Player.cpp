#include "Player.h"
#include <cmath>

Player::Player()
    : speed(300.f)
{
    shape.setSize(sf::Vector2f(50.f, 50.f));
    shape.setFillColor(sf::Color::Blue);
    shape.setOrigin(shape.getSize() / 2.f);
    shape.setPosition(sf::Vector2f(400.f, 300.f));
}

sf::Vector2f Player::Normalize(sf::Vector2f v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);

    if (len == 0.f)
        return sf::Vector2f(0.f, 0.f);

    return sf::Vector2f(v.x / len, v.y / len);
}

void Player::Update(float dt, const sf::Vector2f& worldSize)
{
    sf::Vector2f input(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        input.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        input.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        input.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        input.y += 1.f;

    input = Normalize(input);
    shape.move(input * speed * dt);
}

void Player::Draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

sf::Vector2f Player::GetPosition() const
{
    return shape.getPosition();
}