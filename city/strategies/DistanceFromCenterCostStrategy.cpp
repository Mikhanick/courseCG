#include "DistanceFromCenterCostStrategy.h"
#include "../core/AbstractRoad.h"
#include <QVector2D>
#include <algorithm>
#include <cmath>

namespace City {

DistanceFromCenterCostStrategy::DistanceFromCenterCostStrategy(const QVector2D& cityCenter)
    : m_cityCenter(cityCenter)
{
}

float DistanceFromCenterCostStrategy::calculateCost(const AbstractRoad* road) const {
    if (!road) return 0.5f;

    // Берём центр дороги как среднюю точку
    QVector3D mid = (road->getStart() + road->getEnd()) * 0.5f;
    QVector2D road2d(mid.x(), mid.z());

    float distance = road2d.distanceToPoint(m_cityCenter);

    // Предположим, что максимальное расстояние — 1000 м (можно параметризовать)
    const float maxDistance = 1000.0f;
    float normalizedDist = std::min(distance / maxDistance, 1.0f);

    // Чем ближе к центру — тем дороже: cost = 1 - normalizedDist
    return 1.0f - normalizedDist;
}

} // namespace City