#include "CityMap.h"
#include <numeric>
#include <stdexcept>
#include <iostream>

namespace City {

CityMap::CityMap(
    std::unique_ptr<AbstractRoadGenerationStrategy> roadGen,
    std::unique_ptr<AbstractBuildingSelector> buildingSelector
)
    : m_roadGen(std::move(roadGen))
    , m_buildingSelector(std::move(buildingSelector))
{
    if (!m_roadGen || !m_buildingSelector) {
        throw std::invalid_argument("Road generator and building selector must be non-null");
    }
}

void CityMap::generate(float cityArea) {
    m_cityArea = cityArea;

    m_roads = m_roadGen->generate(cityArea);
    if (m_roads.empty()) return;

    placeBuildingsOnRoads();
}

void CityMap::placeBuildingsOnRoads() {
    for (auto& road : m_roads) {
        std::vector<std::pair<QRectF, int>> plots;
        road->divideIntoPlots(plots);

        if (plots.empty()) continue;

        for (auto& [area, plotCount] : plots) {
            GraphicObject building = m_buildingSelector->select(area);

            // Определяем размер здания (для позиционирования)
            QVector3D size(0, 0, 0);
            if (!building.points.empty()) {
                for (const auto& p : building.points) {
                    size.setX(qMax(size.x(), p.x()));
                    size.setY(qMax(size.y(), p.y()));
                    size.setZ(qMax(size.z(), p.z()));
                }
            }

            QVector3D globalPos = road->calculateGlobalPosition(area, size);
            QVector3D norm = road->calculateNormal();

            building.placeAt(globalPos, norm);
            building.ComputeFaceNormals();
            road->addBuildingMesh(std::move(building));
        }
    }
}

std::vector<GraphicObject> CityMap::exportToScene() const {
    std::vector<GraphicObject> result;
    for (const auto& road : m_roads) {
        result.push_back(road->getRoadMesh());
        const auto& buildings = road->getBuildingMeshes();
        result.insert(result.end(), buildings.begin(), buildings.end());
    }
    return result;
}

} // namespace City