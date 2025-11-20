#include "GraphicObject.h"
#include <QMatrix4x4>
#include <QtMath>

void GraphicObject::AddPoint(const QVector3D& p) {
    points.push_back(p);
}

void GraphicObject::AddFace(int i0, int i1, int i2, const QColor& color) {
    faces.emplace_back(i0, i1, i2, color);
}

void GraphicObject::ComputeFaceNormals() {
    for (auto& face : faces) {
        if (static_cast<std::size_t>(face.index0) >= points.size() ||
            static_cast<std::size_t>(face.index1) >= points.size() ||
            static_cast<std::size_t>(face.index2) >= points.size()) {
            continue;
        }

        QVector3D p0 = points[face.index0];
        QVector3D p1 = points[face.index1];
        QVector3D p2 = points[face.index2];

        QVector3D edge1 = p1 - p0;
        QVector3D edge2 = p2 - p0;
        face.normal = QVector3D::crossProduct(edge1, edge2).normalized();
    }
}

QVector2D GraphicObject::getDimensions() const {
    if (points.empty()) {
        return QVector2D(0.0f, 0.0f);
    }

    float minX = points[0].x();
    float maxX = points[0].x();
    float minZ = points[0].z();
    float maxZ = points[0].z();

    for (const auto& point : points) {
        minX = qMin(minX, point.x());
        maxX = qMax(maxX, point.x());
        minZ = qMin(minZ, point.z());
        maxZ = qMax(maxZ, point.z());
    }

    float length = maxX - minX;  // размер по оси X (ширина)
    float depth = maxZ - minZ;   // размер по оси Z (глубина/фасад)

    return QVector2D(length, depth);
}

void GraphicObject::placeAt(const QVector3D& coord, const QVector3D& norm) {
    // Нормализуем горизонтальную нормаль (Y = 0)
    QVector3D horizontalNorm(norm.x(), 0.0f, norm.z());
    if (horizontalNorm.length() == 0.0f) return;
    horizontalNorm.normalize();

    // Ось Z исходного здания → направление нормали
    // Ось X исходного здания → перпендикуляр в плоскости XZ
    QVector3D zAxis = horizontalNorm;
    QVector3D yAxis(0, 1, 0);
    QVector3D xAxis = QVector3D::crossProduct(zAxis, yAxis).normalized();

    // Матрица поворота: из локальной системы (Z=фасад, X=вправо) в мировую
    QMatrix4x4 transform;
    transform.setRow(0, QVector4D(xAxis, 0));
    transform.setRow(1, QVector4D(yAxis, 0));
    transform.setRow(2, QVector4D(zAxis, 0));
    transform.setRow(3, QVector4D(0, 0, 0, 1));

    // Применяем трансформацию + сдвиг
    for (auto& p : points) {
        QVector3D local(p.x(), p.y(), p.z());
        QVector3D rotated = transform.map(local);
        p = coord + rotated;
    }
}
