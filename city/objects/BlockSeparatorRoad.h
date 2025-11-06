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

    // Вес для распределения - всегда 0, так как не размещает дома
    float getTypeWeight() const override;

    // Размещение зданий - пустая реализация, не размещает здания
    void divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const override;
    QVector3D calculateGlobalPosition(const QRectF& plot, const QVector3D& buildingSize) const override;
    QVector3D calculateNormal() const override;

    // Экспорт
    GraphicObject getRoadMesh() const override;
    std::vector<GraphicObject> getBuildingMeshes() const override;
    
    // Добавление зданий - игнорируется
    void addBuildingMesh(GraphicObject&& building) override;
    
    // Направление размещения зданий - не используется, всегда 0
    void setBuildingSide(int side) override;
    int getBuildingSide() const override;
    
    // Направление размещения зданий с использованием перечисления
    void setBuildingSideFromEnum(BuildingSide side) override;
    BuildingSide getBuildingSideAsEnum() const override;
    
    // Prototype pattern - clone method
    std::unique_ptr<AbstractRoad> clone() const override;

private:
    QVector3D m_start;
    QVector3D m_end;
    float m_width;
};

} // namespace City

#endif // BLOCKSEPARATORROAD_H