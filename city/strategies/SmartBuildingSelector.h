#pragma once
#include "AbstractBuildingSelector.h"
#include <vector>
#include <random>
#include <QString>
#include "../objects/BuildingModel.h"

namespace City {

class SmartBuildingSelector : public AbstractBuildingSelector {
public:
    explicit SmartBuildingSelector(const QString& modelsDir);
    void setSeed(unsigned int seed);
    
    GraphicObject select(const QRectF& availableArea) override;

private:
    std::vector<BuildingModel> availableModels;
    mutable std::mt19937 randomEngine;
    
    BuildingModel chooseBestModel(const QSizeF& availableSize) const;
    GraphicObject buildFromModel(const BuildingModel& model, const QSizeF& baseSize) const;
    
    // Вспомогательные функции
    QSizeF getBaseDimensions(const FloorSection& section) const;
    float getSectionHeight(const FloorSection& section) const;
};

} // namespace City