#include "SimpleRoadGenerationStrategy.h"
#include "../objects/ResidentialRoad.h"

namespace City {

std::vector<std::unique_ptr<AbstractRoad>> SimpleRoadGenerationStrategy::generate(
    float cityArea)
{
    std::vector<std::unique_ptr<AbstractRoad>> roads;
    
    // Создаем 4 параллельные дороги с увеличенным расстоянием между ними
    // Первая дорога: горизонтальная
    QVector3D startPos1(-150, 0, 0); 
    QVector3D endPos1(150, 0, 0);
    roads.push_back(std::make_unique<ResidentialRoad>(startPos1, endPos1, 12.0f));
    
    // Вторая дорога: параллельна первой, смещенная по оси Z
    QVector3D startPos2(-150, 0, 50); 
    QVector3D endPos2(150, 0, 50);
    roads.push_back(std::make_unique<ResidentialRoad>(startPos2, endPos2, 12.0f));
    
    // Третья дорога: параллельна первой, смещенная по оси Z
    QVector3D startPos3(-150, 0, 100); 
    QVector3D endPos3(150, 0, 100);
    roads.push_back(std::make_unique<ResidentialRoad>(startPos3, endPos3, 12.0f));
    
    // Четвертая дорога: перпендикулярна первым трем, по оси Z, размещена с краю
    QVector3D startPos4(150, 0, -100); 
    QVector3D endPos4(150, 0, 150);
    roads.push_back(std::make_unique<ResidentialRoad>(startPos4, endPos4, 12.0f));
    
    return roads;
}

} // namespace City