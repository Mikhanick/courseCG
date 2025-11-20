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
    const float edgeBuffer = 10.0f;    // Отступ от концов дороги
    const float sideMargin = 5.0f;    // Отступ от края дороги по ширине
    const float plotDepth = 40.0f;    // Фиксированная глубина всех участков
    
    // Гибкие параметры ширины участков (вдоль дороги)
    const float minPlotWidth = 25.0f; // Минимальная ширина (для узких таунхаусов)
    const float maxPlotWidth = 70.0f; // Максимальная ширина (для длинных домов/особняков)
    
    float roadLength = getLength();
    float buildableStart = edgeBuffer;
    float buildableEnd = roadLength - edgeBuffer;
    
    if (buildableEnd <= buildableStart + minPlotWidth) return;
    
    // Определяем стороны застройки
    std::vector<BuildingSide> sides;
    if (m_buildingSide == BuildingSide::BOTH) 
        sides = {BuildingSide::LEFT, BuildingSide::RIGHT};
    else if (m_buildingSide != BuildingSide::NONE) 
        sides = {m_buildingSide};
    
    // Детерминированный генератор случайных чисел
    auto randFloat = [this](int idx) -> float {
        float val = std::abs(m_start.x() * 1000.0f + 
                           m_start.z() * 100.0f + 
                           m_end.x() * 10.0f + 
                           m_end.z() * 1.0f + 
                           idx);
        return std::fmod(val, 10000.0f) / 10000.0f;
    };

    for (size_t sideIdx = 0; sideIdx < sides.size(); ++sideIdx) {
        BuildingSide side = sides[sideIdx];
        float y_offset = (side == BuildingSide::LEFT) 
                         ? sideMargin 
                         : -(sideMargin + plotDepth);
        
        float currentPos = buildableStart;
        int randBase = static_cast<int>(sideIdx * 1000);
        bool isFirstPlot = true; // Для пропуска первого участка

        while (currentPos < buildableEnd) {
            float remaining = buildableEnd - currentPos;
            if (remaining < minPlotWidth) break;

            // 1. СЛУЧАЙНАЯ ШИРИНА УЧАСТКА (независимо от типа здания)
            float plotWidth = minPlotWidth + randFloat(randBase++) * (maxPlotWidth - minPlotWidth);
            
            // 2. ОГРАНИЧЕНИЕ ПОД ОСТАВШЕЕСЯ ПРОСТРАНСТВО
            if (currentPos + plotWidth > buildableEnd) {
                plotWidth = buildableEnd - currentPos;
                if (plotWidth < minPlotWidth * 0.8f) break; // Минимум 80% от минимального
            }
            
            // 3. ВЫБОР ТИПА ЗДАНИЯ (НЕЗАВИСИМО ОТ РАЗМЕРА!)
            int plotType = 1; // Стандартный дом по умолчанию
            
            // 40% шанс таунхауса в городской зоне
            if (roadLength < 150.0f && randFloat(randBase++) < 0.4f) {
                plotType = 3; // Таунхаус (может быть как узким, так и длинным!)
            } 
            // 20% шанс особняка в пригороде
            else if (roadLength >= 150.0f && randFloat(randBase++) < 0.2f) {
                plotType = 2; // Особняк
            }
            
            // 4. ПРОПУСКАЕМ ПЕРВЫЙ УЧАСТОК НА КАЖДОЙ СТОРОНЕ
            if (!isFirstPlot) {
                plots.emplace_back(
                    QRectF(currentPos, y_offset, plotWidth, plotDepth),
                    plotType
                );
            }
            isFirstPlot = false;
            
            // 5. СЛУЧАЙНЫЙ ПРОМЕЖУТОК (независимо от типа здания)
            float minGap = (plotType == 3) ? 0.5f : 1.5f; // Таунхаусы - меньше промежутки
            float maxGap = (plotType == 3) ? 2.0f : 6.0f;
            float gap = minGap + randFloat(randBase++) * (maxGap - minGap);
            
            currentPos += plotWidth + gap;
        }
    }
}

QVector3D ResidentialRoad::calculateGlobalPosition(
    const QRectF& plot, const QVector3D& buildingSize) const
{
    // 1. Вычисляем направление дороги и нормаль один раз
    QVector3D direction = (m_end - m_start).normalized();
    QVector3D normal = calculateNormal();

    // 2. Центр здания ВДОЛЬ дороги (по X в локальных координатах участка)
    float alongRoadCenter = plot.x() + plot.width() / 2.0f;
    QVector3D basePosition = m_start + direction * alongRoadCenter;

    // 3. Центр здания ПЕРПЕНДИКУЛЯРНО дороге (по Z в локальных координатах)
    // plot.y() - расстояние от края дороги до начала участка
    // plot.height() - глубина участка
    // buildingSize.z - глубина здания (важно для центрирования)
    float perpendicularOffset = plot.y() + (plot.height() - buildingSize.z()) / 2.0f;

    // 4. Финальная позиция с центрированием по высоте
    QVector3D position = basePosition + normal * perpendicularOffset;
    position.setY(buildingSize.y() / 2.0f); // Центр по высоте для правильного вращения

    return position;
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
    QVector3D direction = (m_end - m_start).normalized();
    QVector3D normal = calculateNormal();

    // Высота дороги над землей
    const float roadHeight = 0.2f;
    float roadLength = getLength();

    // Создаем дорогу частями по 20 единиц
    const float segmentLength = 20.0f;
    int numSegments = std::ceil(roadLength / segmentLength);

    // Создаем основную дорогу из сегментов
    for (int i = 0; i < numSegments; ++i) {
        float startT = static_cast<float>(i) / numSegments;
        float endT = static_cast<float>(i + 1) / numSegments;

        QVector3D segStart = m_start + direction * (startT * roadLength);
        QVector3D segEnd = m_start + direction * (endT * roadLength);

        // Четыре угла сегмента дороги (поднятые над землей)
        QVector3D p0 = segStart - normal * halfWidth;
        QVector3D p1 = segStart + normal * halfWidth;
        QVector3D p2 = segEnd + normal * halfWidth;
        QVector3D p3 = segEnd - normal * halfWidth;

        int baseIdx = road.points.size();

        road.AddPoint(QVector3D(p0.x(), roadHeight, p0.z()));
        road.AddPoint(QVector3D(p1.x(), roadHeight, p1.z()));
        road.AddPoint(QVector3D(p2.x(), roadHeight, p2.z()));
        road.AddPoint(QVector3D(p3.x(), roadHeight, p3.z()));

        QColor roadColor(100, 100, 100); // серый цвет для жилой дороги

        road.AddFace(baseIdx + 0, baseIdx + 1, baseIdx + 2, roadColor);
        road.AddFace(baseIdx + 0, baseIdx + 2, baseIdx + 3, roadColor);
    }

    // Добавляем разметку посередине дороги (поверх основной дороги)
    addCenterMarkings(road, direction, normal, roadHeight);

    // Добавляем шестиугольные окончания дороги (с перпендикулярной линией перед ними)
    addHexagonalEndings(road, direction, normal, roadHeight);

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

// Добавляем стоп-линии на концах дороги
void ResidentialRoad::addStopLines(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const {
    float halfWidth = m_width / 2.0f;
    const float stopLineWidth = 0.1f;  // Толщина стоп-линии
    const float stopLineHeight = roadHeight + 0.01f; // Немного выше асфальта

    // Стоп-линия в начале дороги
    QVector3D startLeft = m_start - normal * halfWidth;
    QVector3D startRight = m_start + normal * halfWidth;
    QVector3D startLeftFront = startLeft + direction * stopLineWidth;
    QVector3D startRightFront = startRight + direction * stopLineWidth;

    int baseIdx = road.points.size();

    road.AddPoint(QVector3D(startLeft.x(), stopLineHeight, startLeft.z()));      // 0
    road.AddPoint(QVector3D(startRight.x(), stopLineHeight, startRight.z()));    // 1
    road.AddPoint(QVector3D(startRightFront.x(), stopLineHeight, startRightFront.z())); // 2
    road.AddPoint(QVector3D(startLeftFront.x(), stopLineHeight, startLeftFront.z()));   // 3

    road.AddFace(baseIdx + 0, baseIdx + 1, baseIdx + 2, QColor(255, 255, 255)); // белый цвет
    road.AddFace(baseIdx + 0, baseIdx + 2, baseIdx + 3, QColor(255, 255, 255));

    // Стоп-линия в конце дороги
    QVector3D endLeft = m_end - normal * halfWidth;
    QVector3D endRight = m_end + normal * halfWidth;
    QVector3D endLeftBack = endLeft - direction * stopLineWidth;
    QVector3D endRightBack = endRight - direction * stopLineWidth;

    baseIdx = road.points.size();

    road.AddPoint(QVector3D(endLeftBack.x(), stopLineHeight, endLeftBack.z()));  // 0
    road.AddPoint(QVector3D(endRightBack.x(), stopLineHeight, endRightBack.z())); // 1
    road.AddPoint(QVector3D(endRight.x(), stopLineHeight, endRight.z()));         // 2
    road.AddPoint(QVector3D(endLeft.x(), stopLineHeight, endLeft.z()));          // 3

    road.AddFace(baseIdx + 0, baseIdx + 1, baseIdx + 2, QColor(255, 255, 255)); // белый цвет
    road.AddFace(baseIdx + 0, baseIdx + 2, baseIdx + 3, QColor(255, 255, 255));
}

// Добавляем разметку посередине дороги
void ResidentialRoad::addCenterMarkings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const {
    float halfWidth = m_width / 2.0f;
    float roadLength = getLength();

    const float markingWidth = m_width * 0.1f; // Ширина разметки - 10% от ширины дороги
    const float markingHeight = roadHeight + 0.01f; // Немного выше асфальта
    const float markingLength = 5.0f; // Длина одного сегмента разметки
    const float gapLength = 5.0f;    // Промежуток между сегментами

    QVector3D roadCenter = m_start + normal * 0.0f; // Центральная линия дороги

    float totalMarkingLength = markingLength + gapLength;
    int numMarkings = static_cast<int>(roadLength / totalMarkingLength);

    for (int i = 0; i < numMarkings; ++i) {
        float startPos = static_cast<float>(i) * totalMarkingLength + gapLength / 2.0f; // Начинаем с промежутка
        float endPos = startPos + markingLength;

        // Ограничиваем позиции длиной дороги
        if (startPos >= roadLength) break;
        if (endPos > roadLength) endPos = roadLength;

        QVector3D markingStart = roadCenter + direction * startPos;
        QVector3D markingEnd = roadCenter + direction * endPos;

        QVector3D halfNormal = normal * (markingWidth / 2.0f);

        QVector3D p0 = markingStart - halfNormal;
        QVector3D p1 = markingStart + halfNormal;
        QVector3D p2 = markingEnd + halfNormal;
        QVector3D p3 = markingEnd - halfNormal;

        int baseIdx = road.points.size();

        road.AddPoint(QVector3D(p0.x(), markingHeight, p0.z()));
        road.AddPoint(QVector3D(p1.x(), markingHeight, p1.z()));
        road.AddPoint(QVector3D(p2.x(), markingHeight, p2.z()));
        road.AddPoint(QVector3D(p3.x(), markingHeight, p3.z()));

        road.AddFace(baseIdx + 0, baseIdx + 1, baseIdx + 2, QColor(255, 255, 255)); // белый цвет
        road.AddFace(baseIdx + 0, baseIdx + 2, baseIdx + 3, QColor(255, 255, 255));
    }
}


// Добавляем шестиугольные окончания дороги
void ResidentialRoad::addHexagonalEndings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const {
    const float radius = m_width / 2.0f; // Радиус окончания (для плавного перехода)
    const int sides = 6; // Количество сторон у шестиугольного окончания

    // Добавляем стоп-линию и окончание в начале дороги
    addStopLineAndRoundedEnding(road, m_start, direction, normal, roadHeight, radius, sides, true);

    // Добавляем стоп-линию и окончание в конце дороги
    addStopLineAndRoundedEnding(road, m_end, direction, normal, roadHeight, radius, sides, false);
}

// Добавляем стоп-линию и округлое окончание дороги
void ResidentialRoad::addStopLineAndRoundedEnding(GraphicObject& road, const QVector3D& position, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const {
    const float lineWidth = 0.1f;  // Толщина линии
    const float lineHeight = roadHeight + 0.01f; // Немного выше асфальта
    const float stopLineSize = m_width * 0.8f; // Ширина стоп-линии (меньше ширины дороги)

    // Позиция для стоп-линии (немного до/после центра окончания)
    QVector3D stopLineCenter = position;
    if (isStart) {
        // Для начала дороги - немного смещаем вперед
        stopLineCenter = position + direction * (radius * 0.3f);
    } else {
        // Для конца дороги - немного смещаем назад
        stopLineCenter = position - direction * (radius * 0.3f);
    }

    // Вычисляем боковые точки для стоп-линии (используем нормаль как боковой вектор)
    QVector3D leftPoint = stopLineCenter - normal * (stopLineSize / 2.0f);
    QVector3D rightPoint = stopLineCenter + normal * (stopLineSize / 2.0f);
    QVector3D forwardLeft = leftPoint + (isStart ? direction : -direction) * lineWidth;
    QVector3D forwardRight = rightPoint + (isStart ? direction : -direction) * lineWidth;

    // Добавляем точки стоп-линии
    int baseIdx = road.points.size();
    road.AddPoint(QVector3D(leftPoint.x(), lineHeight, leftPoint.z()));      // 0
    road.AddPoint(QVector3D(rightPoint.x(), lineHeight, rightPoint.z()));    // 1
    road.AddPoint(QVector3D(forwardRight.x(), lineHeight, forwardRight.z())); // 2
    road.AddPoint(QVector3D(forwardLeft.x(), lineHeight, forwardLeft.z()));   // 3

    road.AddFace(baseIdx + 0, baseIdx + 1, baseIdx + 2, QColor(255, 255, 255)); // белый цвет
    road.AddFace(baseIdx + 0, baseIdx + 2, baseIdx + 3, QColor(255, 255, 255));

    // Добавляем полушестиугольное (округлое) окончание
    addRoundedEndingAtPosition(road, position, direction, normal, roadHeight, radius, sides, isStart);
}

// Вспомогательный метод для добавления полушестиугольного (округлого) окончания в определенной позиции
void ResidentialRoad::addRoundedEndingAtPosition(GraphicObject& road, const QVector3D& center, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const {
    // Вычисляем боковой вектор (перпендикуляр к направлению дороги)
    // Вместо перпендикуляра к нормали и направлению, просто используем нормаль как боковой вектор
    QVector3D sideVector = normal;

    // Для полушестиугольника используем только половину точек
    std::vector<QVector3D> halfHexagonPoints;
    const float angleStep = M_PI / (sides / 2); // Половина от полного круга для полушестиугольника

    for (int i = 0; i <= sides / 2; ++i) { // Включаем середину (sides/2 + 1 всего точек)
        float angle = (isStart ? M_PI : 0) + (i * angleStep); // Начинаем с другой стороны для начала/конца
        // Создаем точку в плоскости, перпендикулярной направлению дороги
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        // Преобразуем точку в мировые координаты
        QVector3D point = center + sideVector * x + direction * y;
        point.setY(roadHeight);

        halfHexagonPoints.push_back(point);
    }

    // Создаем полушестиугольник из треугольников (с центральной точкой)
    QVector3D centerPoint = center;
    centerPoint.setY(roadHeight);
    int centerIdx = road.points.size();
    road.AddPoint(centerPoint);

    // Сохраняем начальный индекс для точек полушестиугольника
    int hexStartIdx = road.points.size();

    // Добавляем точки полушестиугольника
    for (const auto& point : halfHexagonPoints) {
        road.AddPoint(point);
    }

    // Добавляем треугольники для формирования полушестиугольника
    for (int i = 0; i < static_cast<int>(halfHexagonPoints.size()) - 1; ++i) {
        int currentIdx = hexStartIdx + i;
        int nextIdx = hexStartIdx + i + 1;

        // Добавляем треугольник: центр - текущая точка - следующая точка
        road.AddFace(centerIdx, currentIdx, nextIdx, QColor(100, 100, 100)); // серый цвет для жилой дороги
    }
}

} // namespace City