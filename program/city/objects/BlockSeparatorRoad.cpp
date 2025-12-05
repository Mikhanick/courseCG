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

// Размещение зданий - пустая реализация, не размещает здания
void BlockSeparatorRoad::divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const {
    plots.clear(); // Не создаем участки для строительства
}

QVector3D BlockSeparatorRoad::calculateGlobalPosition(
    const QRectF& plot, const QVector3D& buildingSize, bool isLeftSide) const
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
    QVector3D direction = (m_end - m_start).normalized();
    QVector3D normal = calculateNormal();

    // Высота дороги над землей
    const float roadHeight = 0.25f;
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

        road.AddFace(baseIdx + 0, baseIdx + 1, baseIdx + 2, QColor(80, 80, 80)); // темно-серый асфальт для разделительных дорог
        road.AddFace(baseIdx + 0, baseIdx + 2, baseIdx + 3, QColor(80, 80, 80));
    }

    // Добавляем разметку посередине дороги (поверх основной дороги)
    addCenterMarkings(road, direction, normal, roadHeight);

    // Добавляем шестиугольные окончания дороги (с перпендикулярной линией перед ними)
    addHexagonalEndings(road, direction, normal, roadHeight);

    return road;
}

std::vector<GraphicObject> BlockSeparatorRoad::getBuildingMeshes() const {
    return std::vector<GraphicObject>(); // Возвращаем пустой вектор
}

void BlockSeparatorRoad::addBuildingMesh(GraphicObject&& building) {
    // Игнорируем, так как не размещаем зданий
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

// Добавляем стоп-линии на концах дороги
void BlockSeparatorRoad::addStopLines(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const {
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
void BlockSeparatorRoad::addCenterMarkings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const {
    float halfWidth = m_width / 2.0f;
    float roadLength = getLength();

    const float markingWidth = m_width * 0.05f; // Ширина разметки - 5% от ширины дороги (более узкая)
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
void BlockSeparatorRoad::addHexagonalEndings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const {
    const float radius = m_width / 2.0f; // Радиус окончания (для плавного перехода)
    const int sides = 6; // Количество сторон у шестиугольного окончания

    // Добавляем стоп-линию и окончание в начале дороги
    addStopLineAndRoundedEnding(road, m_start, direction, normal, roadHeight, radius, sides, true);

    // Добавляем стоп-линию и окончание в конце дороги
    addStopLineAndRoundedEnding(road, m_end, direction, normal, roadHeight, radius, sides, false);
}

// Добавляем стоп-линию и округлое окончание дороги
void BlockSeparatorRoad::addStopLineAndRoundedEnding(GraphicObject& road, const QVector3D& position, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const {
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
void BlockSeparatorRoad::addRoundedEndingAtPosition(GraphicObject& road, const QVector3D& center, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const {
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
        road.AddFace(centerIdx, currentIdx, nextIdx, QColor(80, 80, 80)); // темно-серый асфальт для разделительной дороги
    }
}

} // namespace City