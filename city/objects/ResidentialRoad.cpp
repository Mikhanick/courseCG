#include "ResidentialRoad.h"
#include <QVector2D>
#include <cmath>
#include <QtMath>
#include <iostream>
#include <random>

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
    const float houseDepth = 40.0f; // максимальная глубина из SimpleBuildingSelector
    
    // Если длина дороги слишком короткая для размещения с отступами, выходим
    if (length <= 2 * houseDepth) return;

    // Доступная длина для застройки с учетом отступов по краям
    float buildableLength = length - 2 * houseDepth;
    if (buildableLength <= 0) return;

    // Определяем глубину застройки в зависимости от стороны размещения
    float totalBuildableDepth = 50.0f; // общая глубина застройки от дороги
    float buildableDepth = totalBuildableDepth;
    
    // Если здания размещаются только с одной стороны, уменьшаем доступную глубину
    // Или если m_buildingSide равно BuildingSide::NONE, не создаем участки
    if (m_buildingSide == BuildingSide::LEFT || m_buildingSide == BuildingSide::RIGHT) {
        buildableDepth = totalBuildableDepth / 2.0f;
    } else if (m_buildingSide == BuildingSide::NONE) {
        return; // Don't create plots if no buildings should be placed
    }
    // If m_buildingSide == BuildingSide::BOTH, keep the full depth

    if (length <= 100.0f) {
        // Для дорог длиной до 100 создаем один участок, занимающий 20-90% длины
        // Участок начинается после отступа от начала дороги
        float minPlotLength = buildableLength * 0.2f; // 20% от доступной длины
        float maxPlotLength = buildableLength * 0.9f; // 90% от доступной длины
        
        // Вычисляем случайную длину участка в заданном диапазоне
        // Для детерминированного результата используем координаты начала дороги
        float deterministicValue = fmod(m_start.x() * 1000.0f + m_start.z(), 1000.0f);
        if (deterministicValue < 0) deterministicValue += 1000.0f; // обеспечиваем положительное значение
        float plotLength = minPlotLength + 
            (static_cast<float>(deterministicValue) / 1000.0f) * 
            (maxPlotLength - minPlotLength);
        
        // Позиция участка - центрируем его в доступной области или используем фиксированное смещение
        float plotStart = houseDepth + (buildableLength - plotLength) / 2.0f;
        
        plots.emplace_back(QRectF(plotStart, 0, plotLength, buildableDepth), 1);
    } else {
        // Для дорог длиной более 100 создаем несколько участков
        // Участки начинаются после отступа от начала дороги
        const float minPlotLength = 20.0f; // минимальный размер участка
        const float gap = 5.0f; // зазор между участками
        
        float currentPos = houseDepth; // начальная позиция после отступа
        
        while (currentPos < houseDepth + buildableLength) {
            float remainingLength = (houseDepth + buildableLength) - currentPos;
            
            if (remainingLength <= minPlotLength) {
                // Если оставшееся пространство слишком мало для нового участка,
                // добавляем его к последнему участку (если он есть)
                if (!plots.empty()) {
                    plots.back().first.setWidth(plots.back().first.width() + remainingLength);
                }
                break;
            }
            
            // Вычисляем длину текущего участка (не больше оставшегося пространства минус зазор)
            float plotLength = qMin(remainingLength - gap, 50.0f); // ограничиваем максимальный размер участка
            if (plotLength < minPlotLength) {
                plotLength = remainingLength; // если оставшееся пространство меньше минимального, используем всё
            }
            
            if (plotLength >= minPlotLength) {
                plots.emplace_back(QRectF(currentPos, 0, plotLength, buildableDepth), 1);
                currentPos += plotLength + gap;
            } else {
                break;
            }
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
    if (m_buildingSide == BuildingSide::RIGHT) { // правая сторона (ранее 1)
        // Используем нормаль как есть (положительное смещение вправо)
    } else if (m_buildingSide == BuildingSide::LEFT) { // левая сторона (ранее -1)
        // Инвертируем нормаль (отрицательное смещение влево)
        normal = -normal;
    }
    // Если m_buildingSide == BuildingSide::BOTH, используем нормаль как есть (обе стороны)
    // Если m_buildingSide == BuildingSide::NONE, здания не должны создаваться, 
    // но если этот метод вызывается, то участок уже создан, поэтому обрабатываем как обе стороны
    
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
    switch (side) {
        case -1:
            m_buildingSide = BuildingSide::LEFT;
            break;
        case 0:
            m_buildingSide = BuildingSide::NONE;
            break;
        case 1:
            m_buildingSide = BuildingSide::RIGHT;
            break;
        case 2:
            m_buildingSide = BuildingSide::BOTH;
            break;
        default:
            // For any other value, default to both sides
            m_buildingSide = BuildingSide::BOTH;
            break;
    }
}

int ResidentialRoad::getBuildingSide() const {
    // Convert the enum to int according to the original mapping
    switch (m_buildingSide) {
        case BuildingSide::LEFT:
            return -1;
        case BuildingSide::NONE:
            return 0;
        case BuildingSide::RIGHT:
            return 1;
        case BuildingSide::BOTH:
            return 2;
        default:
            return 2; // default to both sides
    }
}

void ResidentialRoad::setBuildingSideFromEnum(BuildingSide side) {
    m_buildingSide = side;
}

BuildingSide ResidentialRoad::getBuildingSideAsEnum() const {
    return m_buildingSide;
}

std::unique_ptr<AbstractRoad> ResidentialRoad::clone() const {
    auto newRoad = std::make_unique<ResidentialRoad>(m_start, m_end, m_width);
    // Convert the enum back to int for the setBuildingSide interface
    int sideValue = 0;
    switch (m_buildingSide) {
        case BuildingSide::LEFT:
            sideValue = -1;
            break;
        case BuildingSide::NONE:
            sideValue = 0;
            break;
        case BuildingSide::RIGHT:
            sideValue = 1;
            break;
        case BuildingSide::BOTH:
            sideValue = 2;
            break;
    }
    newRoad->setBuildingSide(sideValue);
    return newRoad;
}

} // namespace City