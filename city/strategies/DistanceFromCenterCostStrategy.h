#ifndef DISTANCEFROMCENTERCOSTSTRATEGY_H
#define DISTANCEFROMCENTERCOSTSTRATEGY_H

#include "AbstractCostStrategy.h"
#include <QVector2D>

namespace City {

class DistanceFromCenterCostStrategy : public AbstractCostStrategy {
public:
    explicit DistanceFromCenterCostStrategy(const QVector2D& cityCenter);

    float calculateCost(const AbstractRoad* road) const override;

private:
    QVector2D m_cityCenter;
};

} // namespace City

#endif // DISTANCEFROMCENTERCOSTSTRATEGY_H