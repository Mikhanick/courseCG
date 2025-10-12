#ifndef SIMPLEROADGENERATIONSTRATEGY_H
#define SIMPLEROADGENERATIONSTRATEGY_H

#include "AbstractRoadGenerationStrategy.h"
#include "../objects/ResidentialRoad.h"
#include <memory>
#include <vector>

namespace City {

class SimpleRoadGenerationStrategy : public AbstractRoadGenerationStrategy {
public:
    SimpleRoadGenerationStrategy() = default;

    /**
     * Генерирует простой город с одной длинной дорогой и зданиями по бокам
     * @param cityArea Общая площадь города (м²)
     * @param totalPopulation Общее население
     * @return Список уникальных указателей на одну дорогу
     */
    std::vector<std::unique_ptr<AbstractRoad>> generate(
        float cityArea, int totalPopulation) override;
};

} // namespace City

#endif // SIMPLEROADGENERATIONSTRATEGY_H