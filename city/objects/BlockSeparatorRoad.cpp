#include "BlockSeparatorRoad.h"
#include <QVector2D>
#include <cmath>
#include <QtMath>
#include <iostream>

namespace City {

BlockSeparatorRoad::BlockSeparatorRoad(const QVector3D& start, const QVector3D& end, float width)
    : m_start(start), m_end(end), m_width(width)
{
}

QVector3D BlockSeparatorRoad::getStart() const { return m_start; }
QVector3D BlockSeparatorRoad::getEnd() const { return m_end; }
float BlockSeparatorRoad::getWidth() const { return m_width; }
float BlockSeparatorRoad::getLength() const {
    return QVector3D(m_end - m_start).length();
}

// Вес всегда 0, так как не размещает дома
float BlockSeparatorRoad::getTypeWeight() const { return 0.0f; }

// Размещение зданий - пустая реализация, не размещает здания
void BlockSeparatorRoad::divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const {
    plots.clear(); // Не создаем участки для строительства
}

QVector3D BlockSeparatorRoad::calculateGlobalPosition(
    const QRectF& plot, const QVector3D& buildingSize) const
{
    // Возвращаем нулевую позицию, так как не используется
    return QVector3D(0, 0, 0);
}

QVector3D BlockSeparatorRoad::calculateNormal() const {
    QVector3D dir = m_end - m_start;
    // Нормаль в плоскости XZ, перпендикулярная дороге
    QVector3D normal(-dir.z(), 0, dir.x());
    if (normal.length() > 0) normal.normalize();
    return normal;
}

GraphicObject BlockSeparatorRoad::getRoadMesh() const {
    GraphicObject road;
    float halfWidth = m_width / 2.0f;
    QVector3D normal = calculateNormal();
    
    // Высота дороги над землей
    const float roadHeight = 0.2f; // 10 см над землей

    // Четыре угла дороги (поднятые над землей)
    QVector3D p0 = m_start - normal * halfWidth;
    QVector3D p1 = m_start + normal * halfWidth;
    QVector3D p2 = m_end + normal * halfWidth;
    QVector3D p3 = m_end - normal * halfWidth;

    road.AddPoint(QVector3D(p0.x(), roadHeight, p0.z()));
    road.AddPoint(QVector3D(p1.x(), roadHeight, p1.z()));
    road.AddPoint(QVector3D(p2.x(), roadHeight, p2.z()));
    road.AddPoint(QVector3D(p3.x(), roadHeight, p3.z()));

    road.AddFace(0, 1, 2, QColor(80, 80, 80)); // темно-серый асфальт для разделительных дорог
    road.AddFace(0, 2, 3, QColor(80, 80, 80));
    return road;
}

std::vector<GraphicObject> BlockSeparatorRoad::getBuildingMeshes() const {
    return std::vector<GraphicObject>(); // Возвращаем пустой вектор
}

void BlockSeparatorRoad::addBuildingMesh(GraphicObject&& building) {
    // Игнорируем, так как не размещаем здания
}

void BlockSeparatorRoad::setBuildingSideFromEnum(BuildingSide side) {
    // Не используется
}

BuildingSide BlockSeparatorRoad::getBuildingSideAsEnum() const {
    return BuildingSide::NONE; // Всегда NONE, так как не размещает здания
}

std::unique_ptr<AbstractRoad> BlockSeparatorRoad::clone() const {
    auto newRoad = std::make_unique<BlockSeparatorRoad>(m_start, m_end, m_width);
    // Since BlockSeparatorRoad doesn't place buildings, we don't need to copy building side info
    newRoad->setBuildingSideFromEnum(City::BuildingSide::NONE); // Set to 0 (NONE) as BlockSeparatorRoad doesn't place buildings
    return newRoad;
}

} // namespace City