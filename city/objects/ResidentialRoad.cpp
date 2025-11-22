#include "ResidentialRoad.h"
#include <QVector2D>
#include <cmath>
#include <QtMath>
#include <iostream>
#include <random>
#include <vector>
#include "../../GlobalRandom.h"  // Include global random functionality

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
    const float edgeBuffer = 18.0f;    // Отступ от концов дороги
    const float sideMargin = 9.0f;    // Отступ от края дороги по ширине

    // Глубина участков теперь случайная между минимум и максимум
    const float minPlotDepth = 15.0f;  // Минимальная глубина участка
    const float maxPlotDepth = 50.0f;  // Максимальная глубина участка

    // Гибкие параметры ширины участков (вдоль дороги)
    const float minPlotWidth = 12.0f; // Минимальная ширина (для узких таунхаусов)
    const float maxPlotWidth = 73.0f; // Максимальная ширина (для длинных домов/особняков)

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

    // Генератор случайных чисел, использующий глобальный генератор
    auto randFloat = []() -> float {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(globalRandomGenerator);
    };

    for (size_t sideIdx = 0; sideIdx < sides.size(); ++sideIdx) {
        BuildingSide side = sides[sideIdx];

        float currentPos = buildableStart;
        bool isFirstPlot = true; // Для пропуска первого участка

        while (currentPos < buildableEnd) {
            float remaining = buildableEnd - currentPos;
            if (remaining < minPlotWidth) break;

            // 1. СЛУЧАЙНАЯ ШИРИНА УЧАСТКА (независимо от типа здания)
            float plotWidth = minPlotWidth + randFloat() * (maxPlotWidth - minPlotWidth);

            // 2. СЛУЧАЙНАЯ ГЛУБИНА УЧАСТКА
            float plotDepth = minPlotDepth + randFloat() * (maxPlotDepth - minPlotDepth);

            // 3. ОГРАНИЧЕНИЕ ПОД ОСТАВШЕЕСЯ ПРОСТРАНСТВО
            if (currentPos + plotWidth > buildableEnd) {
                plotWidth = buildableEnd - currentPos;
                if (plotWidth < minPlotWidth * 0.8f) break; // Минимум 80% от минимального
            }

            // 4. РАСЧЕТ СМЕЩЕНИЯ ДЛЯ ТЕКУЩЕГО УЧАСТКА
            float y_offset = (side == BuildingSide::LEFT)
                             ? sideMargin
                             : -(sideMargin + plotDepth);

            // 5. ПРОПУСКАЕМ ПЕРВЫЙ УЧАСТОК НА КАЖДОЙ СТОРОНЕ
            if (!isFirstPlot) {
                // Используем enum-значения для обозначения стороны
                int sideInfo = (side == BuildingSide::LEFT) ? 1 : 0; // 1 для левой, 0 для правой стороны
                plots.emplace_back(
                    QRectF(currentPos, y_offset, plotWidth, plotDepth),
                    sideInfo
                );
            }
            isFirstPlot = false;

            // 6. СЛУЧАЙНЫЙ ПРОМЕЖУТОК
            float minGap = 7.5f;
            float maxGap = 16.0f;
            float gap = minGap + randFloat() * (maxGap - minGap);

            currentPos += plotWidth + gap;
        }
    }
}
QVector3D ResidentialRoad::calculateGlobalPosition(
    const QRectF& plot, const QVector3D& buildingSize, bool isLeftSide) const
{
    QVector3D direction = (m_end - m_start).normalized();
    QVector3D baseNormal = calculateNormal(); // Направлена ВЛЕВО от дороги

    // 1. Позиция вдоль дороги (X): сохраняем центрирование
    float alongRoadCenter = plot.x() + (plot.width() - buildingSize.x()) / 2.0f;
    
    // 2. ПОЗИЦИЯ ПО ГЛУБИНЕ: ВЫРАВНИВАНИЕ ПО ПЕРЕДНЕЙ КРОМКЕ
    float perpendicularOffset;
    if (isLeftSide) {
        // LEFT: фасад начинается на начале участка (ближайшая к дороге точка)
        perpendicularOffset = plot.y();
    } else {
        // RIGHT: фасад начинается на КОНЦЕ участка (ближайшая к дороге точка)
        perpendicularOffset = plot.y() + plot.height();
    }
    
    // 3. Финальная позиция (без центрирования по глубине!)
    QVector3D basePosition = m_start + direction * alongRoadCenter;
    QVector3D position = basePosition + baseNormal * perpendicularOffset;
    position.setY(buildingSize.y() / 2.0f); // Центр по высоте

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

    // Добавляем деревья вдоль дороги
    std::vector<GraphicObject> trees = generateTreesAlongRoad();
    for (auto& tree : trees) {
        int basePointIdx = road.points.size();
        int baseFaceIdx = road.faces.size();

        // Добавляем точки дерева
        for (const auto& point : tree.points) {
            road.AddPoint(point);
        }

        // Добавляем грани дерева с правильной индексацией
        for (const auto& face : tree.faces) {
            road.AddFace(basePointIdx + face.index0, basePointIdx + face.index1, basePointIdx + face.index2, face.color);
        }
    }

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

GraphicObject ResidentialRoad::generateTree(int numVertices, const std::vector<std::pair<float, float>>& crownLevels, const QColor& downColor, const QColor& upColor) const {
    GraphicObject tree;

    // Цвет ствола
    QColor trunkColor(139, 69, 19); // Коричневый цвет для ствола

    if (crownLevels.empty() || numVertices < 3) {
        return tree;
    }

    // Добавляем вершины для ствола (цилиндр)
    float trunkRadius = crownLevels[0].second * 0.2f; // Радиус ствола - 20% от радиуса нижнего кольца кроны
    float trunkHeight = crownLevels[0].first; // Высота ствола - до нижнего кольца кроны

    const int trunkSides = 8; // Количество сторон для цилиндрического ствола
    std::vector<std::vector<int>> trunkLevels; // Уровни вершин ствола

    // Создаем два уровня для ствола: низ (y=0) и верх (y=trunkHeight)
    std::vector<int> bottomTrunkIndices;
    std::vector<int> topTrunkIndices;

    for (int i = 0; i < trunkSides; ++i) {
        float angle = 2.0f * M_PI * i / trunkSides;
        float x = trunkRadius * cos(angle);
        float z = trunkRadius * sin(angle);

        // Нижний круг ствола (основание)
        int bottomIdx = tree.points.size();
        tree.AddPoint(QVector3D(x, 0.0f, z));
        bottomTrunkIndices.push_back(bottomIdx);

        // Верхний круг ствола
        int topIdx = tree.points.size();
        tree.AddPoint(QVector3D(x, trunkHeight, z));
        topTrunkIndices.push_back(topIdx);
    }
    trunkLevels.push_back(bottomTrunkIndices);
    trunkLevels.push_back(topTrunkIndices);

    // Создаем боковые поверхности ствола
    for (int i = 0; i < trunkSides; ++i) {
        int current = trunkLevels[1][i]; // верхний уровень
        int next = trunkLevels[1][(i + 1) % trunkSides];
        int bottomCurrent = trunkLevels[0][i]; // нижний уровень
        int bottomNext = trunkLevels[0][(i + 1) % trunkSides];

        // Треугольники для боковой поверхности ствола
        tree.AddFace(bottomCurrent, next, current, trunkColor); // треугольник 1
        tree.AddFace(bottomCurrent, bottomNext, next, trunkColor); // треугольник 2
    }

    // Создаем крону дерева
    std::vector<std::vector<int>> crownLevelsIndices; // Индексы вершин для каждого уровня кроны

    for (const auto& level : crownLevels) {
        float height = level.first;
        float radius = level.second;

        std::vector<int> levelIndices;
        for (int i = 0; i < numVertices; ++i) {
            float angle = 2.0f * M_PI * i / numVertices;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            int idx = tree.points.size();
            tree.AddPoint(QVector3D(x, height, z));
            levelIndices.push_back(idx);
        }
        crownLevelsIndices.push_back(levelIndices);
    }

    // Создаем боковые поверхности кроны
    for (size_t level = 0; level < crownLevelsIndices.size() - 1; ++level) {
        const auto& currentLevel = crownLevelsIndices[level];
        const auto& nextLevel = crownLevelsIndices[level + 1];

        // Проверяем, уменьшается ли радиус (верхняя нормаль) или увеличивается (нижняя нормаль)
        float currentRadius = crownLevels[level].second;
        float nextRadius = crownLevels[level + 1].second;

        for (int i = 0; i < numVertices; ++i) {
            int currIdx = currentLevel[i];
            int nextIdx = currentLevel[(i + 1) % numVertices];
            int upperCurrIdx = nextLevel[i];
            int upperNextIdx = nextLevel[(i + 1) % numVertices];

            if (nextRadius <= currentRadius) {
                // Если радиус уменьшается, нормали смотрят наружу (вверх) - используем upColor
                tree.AddFace(currIdx, nextIdx, upperCurrIdx, upColor);      // треугольник 1
                tree.AddFace(nextIdx, upperNextIdx, upperCurrIdx, upColor); // треугольник 2
            } else {
                // Если радиус увеличивается, нормали смотрят внутрь (вниз) - используем downColor
                tree.AddFace(currIdx, nextIdx, upperCurrIdx, downColor);      // треугольник 1
                tree.AddFace(nextIdx, upperNextIdx, upperCurrIdx, downColor); // треугольник 2
            }
        }
    }

    // Создаем верхушку кроны (замыкаем верхний уровень)
    if (!crownLevelsIndices.empty()) {
        const auto& topLevel = crownLevelsIndices.back();

        // Создаем центральную вершину для верха
        int topCenterIdx = tree.points.size();
        float topHeight = crownLevels.back().first;
        tree.AddPoint(QVector3D(0.0f, topHeight, 0.0f));

        // Треугольники, соединяющие верхний уровень с центром
        for (int i = 0; i < numVertices; ++i) {
            int currIdx = topLevel[i];
            int nextIdx = topLevel[(i + 1) % numVertices];

            tree.AddFace(currIdx, nextIdx, topCenterIdx, upColor);
        }
    }

    // Создаем нижнюю часть кроны (замыкаем нижний уровень со стволом)
    if (!crownLevelsIndices.empty()) {
        const auto& bottomLevel = crownLevelsIndices[0];

        // Соединяем нижний уровень кроны с верхним уровнем ствола
        for (int i = 0; i < numVertices; ++i) {
            int crownIdx = bottomLevel[i];
            int nextCrownIdx = bottomLevel[(i + 1) % numVertices];
            int trunkIdx = trunkLevels[1][i]; // верхний уровень ствола
            int nextTrunkIdx = trunkLevels[1][(i + 1) % trunkSides]; // следующий индекс ствола

            // Для соединения кроны со стволом (нормали вниз)
            tree.AddFace(crownIdx, nextTrunkIdx, nextCrownIdx, downColor);
            tree.AddFace(crownIdx, trunkIdx, nextTrunkIdx, downColor);
        }
    }

    return tree;
}

std::vector<GraphicObject> ResidentialRoad::generateTreesAlongRoad() const {
    std::vector<GraphicObject> trees;

    // Проверяем длину дороги
    float roadLength = getLength();
    if (roadLength < 40.0f) { // Если дорога короче 40 метров, деревья не ставим
        return trees;
    }

    // Определяем направление дороги
    QVector3D direction = (m_end - m_start).normalized();
    QVector3D normal = calculateNormal(); // Направление влево от дороги

    // Параметры для размещения деревьев
    const float treeSpacing = 10.0f; // Расстояние между деревьями: 10 метров
    const float edgeBuffer = 10.0f;  // Отступ от начала и конца: 20 метров
    const float treeOffset = m_width / 2.0f + 2.0f; // Отступ от края дороги

    // Вычисляем диапазон, где можно размещать деревья
    float startPlacement = edgeBuffer;
    float endPlacement = roadLength - edgeBuffer;

    // Рассчитываем количество позиций для деревьев
    int numTreePositions = static_cast<int>((endPlacement - startPlacement) / treeSpacing);
    if (numTreePositions <= 0) {
        return trees;
    }

    // Определяем параметры дерева (можно сделать параметрами функции в будущем)
    int numVertices = 8; // Восьмиугольник для кроны
    std::vector<std::pair<float, float>> crownLevels = {
        {3.0f, 1.5f},
        {4.0f, 2.0f},
        {5.0f, 2.3f},
        {7.0f, 2.4f},
        {10.0f, 1.5f},
        {15.f, 0.3f}
    };
    QColor downColor("#bf7c00"); // Зеленый цвет для нижних граней (RGB: 34, 139, 34 - лесной зеленый)
    QColor upColor("#efe4c6");     // Темно-зеленый для верхних граней (RGB: 0, 100, 0 - темно-зеленый)

    // Размещаем деревья с обеих сторон дороги
    for (int i = 3; i < numTreePositions; ++i) {
        float positionAlongRoad = startPlacement + i * treeSpacing;

        // Позиция вдоль дороги
        QVector3D basePosition = m_start + direction * positionAlongRoad;

        // Создаем вариации геометрических размеров для дерева (±20%)
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float sizeVariation = 0.8f + dist(globalRandomGenerator) * 0.4f; 

        // Создаем базовые параметры кроны с вариациями
        std::vector<std::pair<float, float>> variedCrownLevels = {
            {3.0f * sizeVariation, 1.5f * sizeVariation},
            {4.0f * sizeVariation, 2.0f * sizeVariation},
            {5.0f * sizeVariation, 2.3f * sizeVariation},
            {7.0f * sizeVariation, 2.4f * sizeVariation},
            {10.0f * sizeVariation, 1.5f * sizeVariation},
            {15.0f * sizeVariation, 0.3f * sizeVariation}
        };

        // Левые деревья (относительно направления от start к end)
        QVector3D leftTreePos = basePosition + normal * treeOffset;
        GraphicObject leftTree = generateTree(numVertices, variedCrownLevels, downColor, upColor);
        leftTree.placeAt(leftTreePos, normal);
        trees.push_back(std::move(leftTree));

        // Правые деревья (относительно направления от start к end)
        QVector3D rightTreePos = basePosition - normal * treeOffset;
        GraphicObject rightTree = generateTree(numVertices, variedCrownLevels, downColor, upColor);
        rightTree.placeAt(rightTreePos, -normal);
        trees.push_back(std::move(rightTree));
    }

    return trees;
}

} // namespace City