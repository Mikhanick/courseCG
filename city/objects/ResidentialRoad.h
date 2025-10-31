#ifndef RESIDENTIALROAD_H
#define RESIDENTIALROAD_H

#include "../core/AbstractRoad.h"
#include <QVector3D>

namespace City {

class ResidentialRoad : public AbstractRoad {
public:
    ResidentialRoad(const QVector3D& start, const QVector3D& end, float width = 12.0f);

    // Геометрия
    QVector3D getStart() const override;
    QVector3D getEnd() const override;
    float getWidth() const override;
    float getLength() const override;

    // Население
    int getAssignedPopulation() const override;
    void setAssignedPopulation(int pop) override;

    // Вес для распределения
    float getTypeWeight() const override;

    // Размещение зданий
    void divideIntoPlots(std::vector<std::pair<QRectF, int>>& plots) const override;
    QVector3D calculateGlobalPosition(const QRectF& plot, const QVector3D& buildingSize) const override;
    QVector3D calculateNormal() const override;

    // Экспорт
    GraphicObject getRoadMesh() const override;
    std::vector<GraphicObject> getBuildingMeshes() const override;
    
    // Добавление зданий
    void addBuildingMesh(GraphicObject&& building) override;
    
    // Направление размещения зданий
    void setBuildingSide(int side) override;
    int getBuildingSide() const override;
    
    // Prototype pattern - clone method
    std::unique_ptr<AbstractRoad> clone() const override;

private:
    QVector3D m_start;
    QVector3D m_end;
    float m_width;
    int m_assignedPopulation = 0;
    int m_buildingSide = 0; // 0 - обе стороны, 1 - правая, -1 - левая
};

} // namespace City

#endif // RESIDENTIALROAD_H