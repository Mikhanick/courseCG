#ifndef CITYMAP_H
#define CITYMAP_H

#include "../strategies/AbstractRoadGenerationStrategy.h"
#include "../strategies/AbstractCostStrategy.h"
#include "../strategies/AbstractBuildingSelector.h"
#include <memory>
#include <vector>

namespace City {

class CityMap {
public:
    CityMap(
        std::unique_ptr<AbstractRoadGenerationStrategy> roadGen,
        std::unique_ptr<AbstractCostStrategy> costStrategy,
        std::unique_ptr<AbstractBuildingSelector> buildingSelector
    );

    void generate(float cityArea, int totalPopulation);
    std::vector<GraphicObject> exportToScene() const;

private:
    void distributePopulation();
    void placeBuildingsOnRoads();

    std::vector<std::unique_ptr<AbstractRoad>> m_roads;
    std::unique_ptr<AbstractRoadGenerationStrategy> m_roadGen;
    std::unique_ptr<AbstractCostStrategy> m_costStrategy;
    std::unique_ptr<AbstractBuildingSelector> m_buildingSelector;

    float m_cityArea = 0.0f;
    int m_totalPopulation = 0;
};

} // namespace City

#endif // CITYMAP_H