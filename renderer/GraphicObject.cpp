#include "GraphicObject.h"

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
