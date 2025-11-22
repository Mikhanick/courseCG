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

        for (auto& [area, plotSideInfo] : plots) {
            GraphicObject building = m_buildingSelector->select(area);
            QVector2D dimensions = building.getDimensions();
            QVector3D size(dimensions.x(), 0, dimensions.y());

            bool isLeftSide = (plotSideInfo == 1); // 1=LEFT, 0=RIGHT
            QVector3D globalPos = road->calculateGlobalPosition(area, size, isLeftSide);
            
            // 3. Правильная нормаль: ДЛЯ ПРАВОЙ СТОРОНЫ ИНВЕРТИРУЕМ
            QVector3D norm = road->calculateNormal();
            if (!isLeftSide) { // RIGHT side
                norm = -norm; // Теперь norm смотрит НА ДОРОГУ для обеих сторон
            }

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