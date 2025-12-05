#ifndef BLOCKSEPARATORROAD_H
#define BLOCKSEPARATORROAD_H

#include "../core/AbstractRoad.h"
#include <QVector3D>

namespace City {

class BlockSeparatorRoad : public AbstractRoad {
public:
    BlockSeparatorRoad(const QVector3D& start, const QVector3D& end, float width = 20.0f);

    // Геометрия
    QVector3D getStart() const override;
    QVector3D getEnd() const override;
    float getWidth() const override;
    float getLength() const override;

    // Тип дороги
    RoadType getType() const override { return RoadType::BLOCK_SEPARATOR; }

    // Размещение зданий - пустая реализация, не размещает здания
    void divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const override;
    QVector3D calculateGlobalPosition(const QRectF& plot, const QVector3D& buildingSize, bool isLeftSide = true) const override;
    QVector3D calculateNormal() const override;

    // Экспорт
    GraphicObject getRoadMesh() const override;
    std::vector<GraphicObject> getBuildingMeshes() const override;

    // Добавление зданий - игнорируется
    void addBuildingMesh(GraphicObject&& building) override;


    virtual void setBuildingSideFromEnum(BuildingSide side) override;
    virtual BuildingSide getBuildingSideAsEnum() const override;

    // Prototype pattern - clone method
    std::unique_ptr<AbstractRoad> clone() const override;

private:
    QVector3D m_start;
    QVector3D m_end;
    float m_width;

    // Внутренние методы для добавления элементов дороги
    void addStopLines(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const;
    void addCenterMarkings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const;
    void addHexagonalEndings(GraphicObject& road, const QVector3D& direction, const QVector3D& normal, float roadHeight) const;
    void addStopLineAndRoundedEnding(GraphicObject& road, const QVector3D& position, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const;
    void addRoundedEndingAtPosition(GraphicObject& road, const QVector3D& center, const QVector3D& direction, const QVector3D& normal, float roadHeight, float radius, int sides, bool isStart) const;
};

} // namespace City

#endif // BLOCKSEPARATORROAD_H