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
};