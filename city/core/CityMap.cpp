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

            // Using the new getDimensions method to get the actual building dimensions
            QVector2D dimensions = building.getDimensions();
            QVector3D size(dimensions.x(), 0, dimensions.y()); // x = length, y = height (not used for positioning), z = depth

            QVector3D globalPos = road->calculateGlobalPosition(area, size);
            QVector3D norm = road->calculateNormal();

            // To center the building in its plot, we need to offset the position to account for
            // the building's dimensions since placeAt uses the building's local (0,0,0) as reference
            // The building is initially positioned with (0,0,0) as the corner, so we need to move it
            // to center it within its allocated plot
            QVector3D dir = (road->getEnd() - road->getStart()).normalized();
            QVector3D perp = QVector3D::crossProduct(QVector3D(0, 1, 0), dir).normalized(); // perpendicular to road direction

            // Adjust global position to center the building in its plot
            // Move by half of each dimension in the appropriate directions
            globalPos = globalPos - (norm * size.z() / 2.0f) - (dir * size.x() / 2.0f);

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