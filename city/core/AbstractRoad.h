#ifndef ABSTRACTROAD_H
#define ABSTRACTROAD_H

#include "../../renderer/GraphicObject.h"
#include <QRectF>
#include <vector>
#include <utility>

namespace City {

class AbstractRoad {
public:
    virtual ~AbstractRoad() = default;

    // Геометрия
    virtual QVector3D getStart() const = 0;
    virtual QVector3D getEnd() const = 0;
    virtual float getWidth() const = 0;
    virtual float getLength() const = 0;

    // Население
    virtual int getAssignedPopulation() const = 0;
    virtual void setAssignedPopulation(int pop) = 0;

    // Вес для распределения населения (например, 1.0 для жилых, 0.3 для магистралей)
    virtual float getTypeWeight() const = 0;

    // Размещение зданий
    virtual void divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const = 0;
    virtual QVector3D calculateGlobalPosition(const QRectF& plot, const QVector3D& buildingSize) const = 0;
    virtual QVector3D calculateNormal() const = 0;

    // Экспорт
    virtual GraphicObject getRoadMesh() const = 0;
    virtual std::vector<GraphicObject> getBuildingMeshes() const = 0;
    
    // Добавление зданий
    virtual void addBuildingMesh(GraphicObject&& building) = 0;

protected:
    std::vector<GraphicObject> m_buildingMeshes;
};

} // namespace City

#endif // ABSTRACTROAD_H