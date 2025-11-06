#ifndef ABSTRACTBUILDINGSELECTOR_H
#define ABSTRACTBUILDINGSELECTOR_H

#include "../renderer/GraphicObject.h"
#include <QRectF>

namespace City {

class AbstractBuildingSelector {
public:
    virtual ~AbstractBuildingSelector() = default;

    /**
     * @param availableArea Прямоугольник застройки (x = длина вдоль дороги, y = глубина от дороги)
     * @return Готовый GraphicObject (координаты >=0, фасад по X, (0,0,0) — левый нижний угол фасада)
     */
    virtual GraphicObject select(const QRectF& availableArea) = 0;
};

} // namespace City

#endif // ABSTRACTBUILDINGSELECTOR_H