#include "CameraController.h"
#include <algorithm>

CameraController::CameraController(const sf::Vector2f& viewSize)
    : m_view(viewSize * 0.5f, viewSize)
{
}

void CameraController::Update(const sf::Vector2f& targetPosition, const sf::Vector2f& worldSize)
{
    sf::Vector2f center = targetPosition;
    sf::Vector2f halfView(m_view.getSize().x * 0.5f, m_view.getSize().y * 0.5f);

    center.x = Clamp(center.x, halfView.x, worldSize.x - halfView.x);
    center.y = Clamp(center.y, halfView.y, worldSize.y - halfView.y);

    m_view.setCenter(center);
}

void CameraController::Apply(sf::RenderWindow& window) const
{
    window.setView(m_view);
}

float CameraController::Clamp(float value, float minValue, float maxValue) const
{
    return std::max(minValue, std::min(value, maxValue));
}