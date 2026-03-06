#pragma once

#include <SFML/Graphics.hpp>

class CameraController
{
public:
    CameraController(const sf::Vector2f& viewSize);

    void Update(const sf::Vector2f& targetPosition, const sf::Vector2f& worldSize);
    void Apply(sf::RenderWindow& window) const;

private:
    float Clamp(float value, float minValue, float maxValue) const;

private:
    sf::View m_view;
};