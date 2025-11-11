#include "ResidentialRoad.h"
#include <QVector2D>
#include <cmath>
#include <QtMath>
#include <iostream>
#include <random>
#include <vector>

namespace City {

ResidentialRoad::ResidentialRoad(const QVector3D& start, const QVector3D& end, float width)
    : m_start(start), m_end(end), m_width(width)
{
}

QVector3D ResidentialRoad::getStart() const { return m_start; }
QVector3D ResidentialRoad::getEnd() const { return m_end; }
float ResidentialRoad::getWidth() const { return m_width; }
float ResidentialRoad::getLength() const {
    return QVector3D(m_end - m_start).length();
}
float ResidentialRoad::getTypeWeight() const { return 1.0f; }

void ResidentialRoad::divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const {
    plots.clear();
    float length = getLength();
    
    // Глубина дома, используемая для отступов по краям
    const float houseDepth = 15.0f;
    
    // Если длина дороги слишком короткая для размещения с отступами, выходим
    if (length <= 2 * houseDepth) return;

    // Доступная длина для застройки с учетом отступов по краям
    float buildableLength = length - 2 * houseDepth;
    if (buildableLength <= 0) return;

    // Глубина застройки на КАЖДУЮ сторону (максимальная глубина из SimpleBuildingSelector)
    const float depthPerSide = 40.0f; // 50.0f total / 2 sides
    
    if (m_buildingSide == BuildingSide::NONE) {
        return;
    }

    // Определяем стороны для застройки
    std::vector<BuildingSide> sidesToBuild;
    if (m_buildingSide == BuildingSide::BOTH) {
        sidesToBuild = {BuildingSide::LEFT, BuildingSide::RIGHT};
    } else {
        sidesToBuild = {m_buildingSide};
    }

    for (BuildingSide side : sidesToBuild) {
        float y_offset = 0.0f;
        if (side == BuildingSide::LEFT) {
            // ИЗМЕНЕНО: для левой стороны теперь 0.0f (поменяли местами отступы)
            y_offset = 0.f;
        } else if (side == BuildingSide::RIGHT) {
            // ИЗМЕНЕНО: для правой стороны теперь -depthPerSide (поменяли местами отступы)
            y_offset = -depthPerSide;
        }

        if (length <= 100.0f) {
            // Для коротких дорог (<=100) создаем один участок
            float minPlotLength = buildableLength * 0.2f;
            float maxPlotLength = buildableLength * 0.9f;
            
            float deterministicValue = fmod(m_start.x() * 1000.0f + m_start.z(), 1000.0f);
            if (deterministicValue < 0) deterministicValue += 1000.0f;
            float plotLength = minPlotLength + 
                (static_cast<float>(deterministicValue) / 1000.0f) * 
                (maxPlotLength - minPlotLength);
            
            float plotStart = houseDepth + (buildableLength - plotLength) / 2.0f;
            
            plots.emplace_back(QRectF(plotStart, y_offset, plotLength, depthPerSide), 1);
        } else {
            // Для длинных дорог (>100) создаем несколько участков
            const float minPlotLength = 20.0f;
            const float gap = 1.0f;
            
            float currentPos = houseDepth;
            
            while (currentPos < houseDepth + buildableLength) {
                float remainingLength = (houseDepth + buildableLength) - currentPos;
                if (remainingLength < minPlotLength) break;
                
                float plotLength = qMin(remainingLength, 50.0f);
                if (remainingLength > minPlotLength + gap) {
                    plotLength = qMin(plotLength, remainingLength - gap);
                }
                
                if (plotLength >= minPlotLength) {
                    plots.emplace_back(QRectF(currentPos, y_offset, plotLength, depthPerSide), 1);
                    currentPos += plotLength + gap;
                } else {
                    break;
                }
            }
        }
    }
}

QVector3D ResidentialRoad::calculateGlobalPosition(
    const QRectF& plot, const QVector3D& buildingSize) const
{
    QVector3D dir = (m_end - m_start).normalized();
    float t = (plot.x() + plot.width() / 2.0f) / getLength();
    QVector3D pos = m_start + dir * (t * getLength());

    // Смещение вбок: используем y-координату участка для определения стороны
    // ИЗМЕНЕНО: отступ увеличен до 10 единиц (было 2.0f)
    float lateral = plot.y() + buildingSize.z() / 2.0f + 10.0f;
    QVector3D normal = calculateNormal();
    
    QVector3D result = pos + normal * lateral;
    result.setY(0.0f);
    return result;
}

QVector3D ResidentialRoad::calculateNormal() const {
    QVector3D dir = m_end - m_start;
    QVector3D normal(-dir.z(), 0, dir.x());
    if (normal.length() > 0) normal.normalize();
    return normal;
}

GraphicObject ResidentialRoad::getRoadMesh() const {
    GraphicObject road;
    float halfWidth = m_width / 2.0f;
    QVector3D normal = calculateNormal();
    
    const float roadHeight = 0.2f;

    QVector3D p0 = m_start - normal * halfWidth;
    QVector3D p1 = m_start + normal * halfWidth;
    QVector3D p2 = m_end + normal * halfWidth;
    QVector3D p3 = m_end - normal * halfWidth;

    road.AddPoint(QVector3D(p0.x(), roadHeight, p0.z()));
    road.AddPoint(QVector3D(p1.x(), roadHeight, p1.z()));
    road.AddPoint(QVector3D(p2.x(), roadHeight, p2.z()));
    road.AddPoint(QVector3D(p3.x(), roadHeight, p3.z()));

    QColor roadColor(100, 100, 100);

    road.AddFace(0, 1, 2, roadColor);
    road.AddFace(0, 2, 3, roadColor);
    return road;
}

std::vector<GraphicObject> ResidentialRoad::getBuildingMeshes() const {
    return m_buildingMeshes;
}

void ResidentialRoad::addBuildingMesh(GraphicObject&& building) {
    m_buildingMeshes.push_back(std::move(building));
}

void ResidentialRoad::setBuildingSideFromEnum(BuildingSide side) {
    m_buildingSide = side;
}

BuildingSide ResidentialRoad::getBuildingSideAsEnum() const {
    return m_buildingSide;
}

std::unique_ptr<AbstractRoad> ResidentialRoad::clone() const {
    auto newRoad = std::make_unique<ResidentialRoad>(m_start, m_end, m_width);
    newRoad->setBuildingSideFromEnum(m_buildingSide);
    return newRoad;
}

} // namespace City