#include "ResidentialRoad.h"
#include <QVector2D>
#include <cmath>
#include <QtMath>
#include <iostream>

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
int ResidentialRoad::getAssignedPopulation() const { return m_assignedPopulation; }
void ResidentialRoad::setAssignedPopulation(int pop) { m_assignedPopulation = pop; }
float ResidentialRoad::getTypeWeight() const { return 1.0f; }

void ResidentialRoad::divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const {
    plots.clear();
    float length = getLength();
    if (length < 40.0f) return; // слишком короткая дорога

    // Доступная зона застройки: отступы от перекрёстков
    float buildableLength = length - 40.0f; // 10 м с каждого конца
    if (buildableLength <= 0) return;

    float buildableDepth = 50.0f; // глубина застройки от дороги

    // Простая упаковка: участки по 200 м в длину с большим зазором между ними
    const float plotLength = 200.0f;
    const float gap = 50.0f; // Увеличенный зазор между участками (было 3.0f)
    float currentPos = 20.0f; // начальный отступ

    int totalPlots = static_cast<int>(buildableLength / (plotLength + gap));
    if (totalPlots <= 0) totalPlots = 1;

    for (int i = 0; i < totalPlots; ++i) {
        float x = currentPos;
        float w = qMin(plotLength, buildableLength - (currentPos - 10.0f));
        if (w <= 5.0f) break;

        plots.emplace_back(QRectF(x, 0, w, buildableDepth), 1); // population placeholder
        currentPos += w + gap;
    }
    
    // Если здания должны строиться только с одной стороны, уменьшаем доступную глубину
    if (m_buildingSide != 0) {
        for (auto& plot : plots) {
            plot.first.setHeight(plot.first.height() / 2.0f); // только половина глубины
        }
    }
}

QVector3D ResidentialRoad::calculateGlobalPosition(
    const QRectF& plot, const QVector3D& buildingSize) const
{
    QVector3D dir = (m_end - m_start).normalized();
    float t = (plot.x() + plot.width() / 2.0f) / getLength(); // центр участка
    QVector3D pos = m_start + dir * (t * getLength());

    // Смещение вбок (глубина здания / 2 + дополнительный зазор)
    float lateral = plot.y() + buildingSize.z() / 2.0f + 2.0f; // Добавляем 2 метра зазора
    QVector3D normal = calculateNormal();
    
    // Если здания должны строиться только с одной стороны, корректируем смещение
    if (m_buildingSide == 1) { // правая сторона
        // Используем нормаль как есть (положительное смещение вправо)
    } else if (m_buildingSide == -1) { // левая сторона
        // Инвертируем нормаль (отрицательное смещение влево)
        normal = -normal;
    }
    // Если m_buildingSide == 0, используем нормаль как есть (обе стороны)
    
    QVector3D result = pos + normal * lateral;
    
    // Устанавливаем высоту здания на уровне земли (Y=0)
    result.setY(0.0f);
    return result;
}

QVector3D ResidentialRoad::calculateNormal() const {
    QVector3D dir = m_end - m_start;
    // Нормаль в плоскости XZ, перпендикулярная дороге
    QVector3D normal(-dir.z(), 0, dir.x());
    if (normal.length() > 0) normal.normalize();
    return normal;
}

GraphicObject ResidentialRoad::getRoadMesh() const {
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

    road.AddFace(0, 1, 2, QColor(100, 100, 100)); // серый асфальт
    road.AddFace(0, 2, 3, QColor(100, 100, 100));
    return road;
}

std::vector<GraphicObject> ResidentialRoad::getBuildingMeshes() const {
    return m_buildingMeshes;
}

void ResidentialRoad::addBuildingMesh(GraphicObject&& building) {
    m_buildingMeshes.push_back(std::move(building));
}

void ResidentialRoad::setBuildingSide(int side) {
    m_buildingSide = side;
}

int ResidentialRoad::getBuildingSide() const {
    return m_buildingSide;
}

std::unique_ptr<AbstractRoad> ResidentialRoad::clone() const {
    auto newRoad = std::make_unique<ResidentialRoad>(m_start, m_end, m_width);
    newRoad->setAssignedPopulation(m_assignedPopulation);
    newRoad->setBuildingSide(m_buildingSide);
    return newRoad;
}

} // namespace City