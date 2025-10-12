#include "SimpleBuildingSelector.h"
#include <QColor>
#include <algorithm>

namespace City {

GraphicObject SimpleBuildingSelector::select(
    const QRectF& availableArea, int population, float cost)
{
    float width = static_cast<float>(availableArea.width()) * 0.8;
    float depth = static_cast<float>(availableArea.height());

    // Определяем тип здания по стоимости и населению
    BuildingType type;

    if (cost > 0.7f && population > 200) {
        type = BuildingType::High;
    } else if (cost > 0.4f || population > 80) {
        type = BuildingType::Mid;
    } else {
        type = BuildingType::Low;
    }

    // Ограничиваем размеры
    width = std::max(10.0f, std::min(width, 40.0f));
    depth = std::max(10.0f, std::min(depth, 30.0f));

    float height;
    switch (type) {
        case BuildingType::High:  height = 30.0f + cost * 20.0f; break;
        case BuildingType::Mid:   height = 12.0f + cost * 8.0f;  break;
        default:                  height = 6.0f + cost * 4.0f;   break;
    }

    switch (type) {
        case BuildingType::High:  return createHighRise(width, depth, height);
        case BuildingType::Mid:   return createMidRise(width, depth, height);
        default:                  return createLowRise(width, depth, height);
    }
}

GraphicObject SimpleBuildingSelector::createLowRise(float width, float depth, float height) const {
    GraphicObject obj;
    QColor color(180, 160, 100); // бежевый

    // Нижнее основание (Y=0)
    obj.AddPoint(QVector3D(0, 0, 0));
    obj.AddPoint(QVector3D(width, 0, 0));
    obj.AddPoint(QVector3D(width, 0, depth));
    obj.AddPoint(QVector3D(0, 0, depth));

    // Верхнее основание (Y=height)
    obj.AddPoint(QVector3D(0, height, 0));
    obj.AddPoint(QVector3D(width, height, 0));
    obj.AddPoint(QVector3D(width, height, depth));
    obj.AddPoint(QVector3D(0, height, depth));

    // Фасад (XZ, Y=0..height)
    obj.AddFace(0, 1, 5, color); // перед
    obj.AddFace(0, 5, 4, color);
    obj.AddFace(1, 2, 6, color); // правый
    obj.AddFace(1, 6, 5, color);
    obj.AddFace(2, 3, 7, color); // зад
    obj.AddFace(2, 7, 6, color);
    obj.AddFace(3, 0, 4, color); // левый
    obj.AddFace(3, 4, 7, color);
    obj.AddFace(4, 5, 6, color); // крыша
    obj.AddFace(4, 6, 7, color);

    return obj;
}

GraphicObject SimpleBuildingSelector::createMidRise(float width, float depth, float height) const {
    GraphicObject obj;
    QColor color(100, 120, 180); // голубоватый

    obj.AddPoint(QVector3D(0, 0, 0));
    obj.AddPoint(QVector3D(width, 0, 0));
    obj.AddPoint(QVector3D(width, 0, depth));
    obj.AddPoint(QVector3D(0, 0, depth));
    obj.AddPoint(QVector3D(0, height, 0));
    obj.AddPoint(QVector3D(width, height, 0));
    obj.AddPoint(QVector3D(width, height, depth));
    obj.AddPoint(QVector3D(0, height, depth));

    obj.AddFace(0, 1, 5, color);
    obj.AddFace(0, 5, 4, color);
    obj.AddFace(1, 2, 6, color);
    obj.AddFace(1, 6, 5, color);
    obj.AddFace(2, 3, 7, color);
    obj.AddFace(2, 7, 6, color);
    obj.AddFace(3, 0, 4, color);
    obj.AddFace(3, 4, 7, color);
    obj.AddFace(4, 5, 6, color);
    obj.AddFace(4, 6, 7, color);


    return obj;
}

GraphicObject SimpleBuildingSelector::createHighRise(float width, float depth, float height) const {
    GraphicObject obj;
    QColor color(60, 60, 70); // тёмно-серый

    obj.AddPoint(QVector3D(0, 0, 0));
    obj.AddPoint(QVector3D(width, 0, 0));
    obj.AddPoint(QVector3D(width, 0, depth));
    obj.AddPoint(QVector3D(0, 0, depth));
    obj.AddPoint(QVector3D(0, height, 0));
    obj.AddPoint(QVector3D(width, height, 0));
    obj.AddPoint(QVector3D(width, height, depth));
    obj.AddPoint(QVector3D(0, height, depth));

    obj.AddFace(0, 1, 5, color);
    obj.AddFace(0, 5, 4, color);
    obj.AddFace(1, 2, 6, color);
    obj.AddFace(1, 6, 5, color);
    obj.AddFace(2, 3, 7, color);
    obj.AddFace(2, 7, 6, color);
    obj.AddFace(3, 0, 4, color);
    obj.AddFace(3, 4, 7, color);
    obj.AddFace(4, 5, 6, color);
    obj.AddFace(4, 6, 7, color);


    return obj;
}

} // namespace City