#ifndef RESIDENTIALROAD_H
#define RESIDENTIALROAD_H

#include "../core/AbstractRoad.h"
#include <QVector3D>
#include <vector>
#include <utility>
#include <QtGui/QColor>

namespace City {

class ResidentialRoad : public AbstractRoad {
public:
    ResidentialRoad(const QVector3D& start, const QVector3D& end, float width = 12.0f);

    // Геометрия
    QVector3D getStart() const override;
    QVector3D getEnd() const override;
    float getWidth() const override;
    float getLength() const override;

    // Тип дороги
    RoadType getType() const override { return RoadType::RESIDENTIAL; }

    // Вес для распределения - всегда 1.0 для жилых дорог
    float getTypeWeight() const override;

    // Размещение зданий
    void divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const override;
    QVector3D calculateGlobalPosition(const QRectF& plot, const QVector3D& buildingSize, bool isLeftSide = true) const override;
    QVector3D calculateNormal() const override;

    // Экспорт
    GraphicObject getRoadMesh() const override;
    std::vector<GraphicObject> getBuildingMeshes() const override;

    // Добавление зданий
    void addBuildingMesh(GraphicObject&& building) override;


    void setBuildingSideFromEnum(BuildingSide side);

    BuildingSide getBuildingSideAsEnum() const override;

    // Prototype pattern - clone method
    std::unique_ptr<AbstractRoad> clone() const override;

    // Function to generate a tree with crown polygons
    GraphicObject generateTree(int numVertices, const std::vector<std::pair<float, float>>& crownLevels,
                               const QColor& downColor, const QColor& upColor) const;

    // Function to generate trees along the road
    std::vector<GraphicObject> generateTreesAlongRoad() const;

private:
    QVector3D m_start;
    QVector3D m_end;
    float m_width;
    BuildingSide m_buildingSide = BuildingSide::BOTH; // Default to both sides

    // Внутренние методы для добавления элементов дороги
    void addStopLines(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const;
    void addCenterMarkings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const;
    void addHexagonalEndings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const;
    void addStopLineAndRoundedEnding(GraphicObject& road, const QVector3D& position, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const;
    void addRoundedEndingAtPosition(GraphicObject& road, const QVector3D& center, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const;
};

} // namespace City

#endif // RESIDENTIALROAD_H