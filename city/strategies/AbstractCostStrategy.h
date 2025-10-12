#ifndef ABSTRACTCOSTSTRATEGY_H
#define ABSTRACTCOSTSTRATEGY_H

#include "../core/AbstractRoad.h"

namespace City {

class AbstractCostStrategy {
public:
    virtual ~AbstractCostStrategy() = default;

    /**
     * @param road Дорога, для которой рассчитывается стоимость
     * @return Стоимость участка [0.0 = дешево, 1.0 = дорого]
     */
    virtual float calculateCost(const AbstractRoad* road) const = 0;
};

} // namespace City

#endif // ABSTRACTCOSTSTRATEGY_H