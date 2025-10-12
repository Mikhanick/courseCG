#ifndef SIMPLEBUILDINGSELECTOR_H
#define SIMPLEBUILDINGSELECTOR_H

#include "AbstractBuildingSelector.h"
#include "../../renderer/GraphicObject.h"

namespace City {

class SimpleBuildingSelector : public AbstractBuildingSelector {
private:
    enum class BuildingType { Low, Mid, High };

public:
    GraphicObject select(const QRectF& availableArea, int population, float cost) override;

private:
    GraphicObject createLowRise(float width, float depth, float height) const;
    GraphicObject createMidRise(float width, float depth, float height) const;
    GraphicObject createHighRise(float width, float depth, float height) const;
};

} // namespace City

#endif // SIMPLEBUILDINGSELECTOR_H