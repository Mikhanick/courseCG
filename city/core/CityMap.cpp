#include "CityMap.h"
#include <numeric>
#include <stdexcept>
#include <iostream>

namespace City {

CityMap::CityMap(
    std::unique_ptr<AbstractRoadGenerationStrategy> roadGen,
    std::unique_ptr<AbstractCostStrategy> costStrategy,
    std::unique_ptr<AbstractBuildingSelector> buildingSelector
)
    : m_roadGen(std::move(roadGen))
    , m_costStrategy(std::move(costStrategy))
    , m_buildingSelector(std::move(buildingSelector))
{
    if (!m_roadGen || !m_costStrategy || !m_buildingSelector) {
        throw std::invalid_argument("All strategies must be non-null");
    }
}

void CityMap::generate(float cityArea, int totalPopulation) {
    m_cityArea = cityArea;
    m_totalPopulation = totalPopulation;

    m_roads = m_roadGen->generate(cityArea, totalPopulation);
    if (m_roads.empty()) return;

    distributePopulation();
    placeBuildingsOnRoads();
}

void CityMap::distributePopulation() {
    float totalWeight = 0.0f;
    for (const auto& road : m_roads) {
        totalWeight += road->getLength() * road->getTypeWeight();
    }

    if (totalWeight <= 0.0f) return;

    int remainingPopulation = m_totalPopulation;
    size_t n = m_roads.size();
    for (size_t i = 0; i < n - 1; ++i) {
        float weight = m_roads[i]->getLength() * m_roads[i]->getTypeWeight();
        int pop = static_cast<int>(m_totalPopulation * (weight / totalWeight));
        m_roads[i]->setAssignedPopulation(pop);
        remainingPopulation -= pop;
    }
    // Последней дороге — остаток
    if (!m_roads.empty()) {
        m_roads.back()->setAssignedPopulation(remainingPopulation);
    }
}

void CityMap::placeBuildingsOnRoads() {
    for (auto& road : m_roads) {
        std::vector<std::pair<QRectF, int>> plots;
        road->divideIntoPlots(plots);

        if (plots.empty()) continue;

        int totalPlotPop = std::accumulate(plots.begin(), plots.end(), 0,
            [](int sum, const auto& p) { return sum + p.second; });

        if (totalPlotPop <= 0) totalPlotPop = 1; // избежать деления на ноль

        for (auto& [area, plotPop] : plots) {
            // Пропорционально распределяем население дороги по участкам
            int assignedPop = static_cast<int>(
                static_cast<float>(road->getAssignedPopulation()) * 
                (static_cast<float>(plotPop) / totalPlotPop)
            );

            float cost = m_costStrategy->calculateCost(road.get());
            GraphicObject building = m_buildingSelector->select(area, assignedPop, cost);

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