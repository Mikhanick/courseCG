#ifndef ABSTRACTROADGENERATIONSTRATEGY_H
#define ABSTRACTROADGENERATIONSTRATEGY_H

#include <memory>
#include <vector>
#include "../core/AbstractRoad.h"

namespace City {

class AbstractRoadGenerationStrategy {
public:
    virtual ~AbstractRoadGenerationStrategy() = default;

    /**
     * @param cityArea Общая площадь города (м²)
     * @return Список уникальных указателей на дороги
     */
    virtual std::vector<std::unique_ptr<AbstractRoad>> generate(
        float cityArea) = 0;
};

} // namespace City

#endif // ABSTRACTROADGENERATIONSTRATEGY_H