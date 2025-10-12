#pragma once
#include <vector>
#include <QtGui/QVector3D>
#include "Face.h"

class GraphicObject {
public:
    std::vector<QVector3D> points;
    std::vector<Face> faces;

    void AddPoint(const QVector3D& p);
    void AddFace(int i0, int i1, int i2, const QColor& color);
    void ComputeFaceNormals();

    /**
     * Размещает объект в глобальном пространстве.
     * Предусловие: все точки имеют координаты >= 0, фасад ориентирован по оси X,
     * первая точка = (0, 0, 0).
     * @param coord — глобальная позиция точки (0,0,0) исходного объекта
     * @param norm — нормаль от дороги к зданию (горизонтальная, Y=0)
     */
    void placeAt(const QVector3D& coord, const QVector3D& norm);
};