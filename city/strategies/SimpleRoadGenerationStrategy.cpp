#include "SimpleRoadGenerationStrategy.h"
#include "../objects/ResidentialRoad.h"

namespace City {

std::vector<std::unique_ptr<AbstractRoad>> SimpleRoadGenerationStrategy::generate(
    float cityArea, int totalPopulation)
{
    std::vector<std::unique_ptr<AbstractRoad>> roads;
    
    // Создаем одну длинную дорогу по центру
    // Удлинняем дорогу для размещения большего количества зданий
    QVector3D startPos(-150, 0, 0); // Удлинненная дорога от -80 до +80 метров
    QVector3D endPos(80, 0, 120);
    
    roads.push_back(std::make_unique<ResidentialRoad>(startPos, endPos, 12.0f));
    
    return roads;
}

} // namespace City