#ifndef ABSTRACTROAD_H
#define ABSTRACTROAD_H

#include "../../renderer/GraphicObject.h"
#include <QRectF>
#include <vector>
#include <utility>
#include "../objects/RoadEnums.h"

namespace City {

enum class RoadType {
    RESIDENTIAL,
    BLOCK_SEPARATOR
};

class AbstractRoad {
public:
    virtual ~AbstractRoad() = default;

    // Геометрия
    virtual QVector3D getStart() const = 0;
    virtual QVector3D getEnd() const = 0;
    virtual float getWidth() const = 0;
    virtual float getLength() const = 0;

    // Тип дороги
    virtual RoadType getType() const = 0;

    // Вес для распределения (например, 1.0 для жилых, 0.3 для магистралей)
    virtual float getTypeWeight() const = 0;

    // Размещение зданий
    virtual void divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const = 0;
    virtual QVector3D calculateGlobalPosition(const QRectF& plot, const QVector3D& buildingSize, bool isLeftSide = true) const = 0;
    virtual QVector3D calculateNormal() const = 0;

    // Экспорт
    virtual GraphicObject getRoadMesh() const = 0;
    virtual std::vector<GraphicObject> getBuildingMeshes() const = 0;
    
    // Добавление зданий
    virtual void addBuildingMesh(GraphicObject&& building) = 0;
    
    
    
    // Направление размещения зданий с использованием перечисления
    virtual void setBuildingSideFromEnum(BuildingSide side) = 0;
    virtual BuildingSide getBuildingSideAsEnum() const = 0;
    
    // Prototype pattern - clone method
    virtual std::unique_ptr<AbstractRoad> clone() const = 0;

protected:
    std::vector<GraphicObject> m_buildingMeshes;
};

} // namespace City

#endif // ABSTRACTROAD_H