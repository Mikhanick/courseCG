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
     * @param population Доступное население для размещения
     * @param cost Стоимость участка [0.0, 1.0]
     * @return Готовый GraphicObject (координаты >=0, фасад по X, (0,0,0) — левый нижний угол фасада)
     */
    virtual GraphicObject select(const QRectF& availableArea, int population, float cost) = 0;
};

} // namespace City

#endif // ABSTRACTBUILDINGSELECTOR_H