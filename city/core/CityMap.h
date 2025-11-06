#ifndef CITYMAP_H
#define CITYMAP_H

#include "../strategies/AbstractRoadGenerationStrategy.h"
#include "../strategies/AbstractBuildingSelector.h"
#include <memory>
#include <vector>

namespace City {

class CityMap {
public:
    CityMap(
        std::unique_ptr<AbstractRoadGenerationStrategy> roadGen,
        std::unique_ptr<AbstractBuildingSelector> buildingSelector
    );

    void generate(float cityArea);
    std::vector<GraphicObject> exportToScene() const;

private:
    void placeBuildingsOnRoads();

    std::vector<std::unique_ptr<AbstractRoad>> m_roads;
    std::unique_ptr<AbstractRoadGenerationStrategy> m_roadGen;
    std::unique_ptr<AbstractBuildingSelector> m_buildingSelector;

    float m_cityArea = 0.0f;
};

} // namespace City

#endif // CITYMAP_H