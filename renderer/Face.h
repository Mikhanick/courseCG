#pragma once
#include <QtGui/QVector3D>
#include <QtGui/QColor>

struct Face {
    int index0, index1, index2;
    QVector3D normal;
    QColor color;

    Face(int i0, int i1, int i2, const QColor& c)
        : index0(i0), index1(i1), index2(i2), color(c) {}
};