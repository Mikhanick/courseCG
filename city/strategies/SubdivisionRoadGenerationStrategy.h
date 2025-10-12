#ifndef SUBDIVISIONROADGENERATIONSTRATEGY_H
#define SUBDIVISIONROADGENERATIONSTRATEGY_H

#include "AbstractRoadGenerationStrategy.h"
#include <QRectF>
#include <set>

namespace City {

class SubdivisionRoadGenerationStrategy : public AbstractRoadGenerationStrategy {
public:
    SubdivisionRoadGenerationStrategy(
        float minBlockSize = 100.0f,  // минимальный размер квартала (м)
        int maxDepth = 5              // максимальная глубина рекурсии
    );

    std::vector<std::unique_ptr<AbstractRoad>> generate(
        float cityArea, int totalPopulation) override;

private:
    struct Block {
        QRectF rect;
        int depth;
    };

    void subdivide(std::vector<Block>& blocks, const Block& block);
    std::vector<std::unique_ptr<AbstractRoad>> blocksToRoads(const std::vector<Block>& blocks) const;

    float m_minBlockSize;
    int m_maxDepth;
};

} // namespace City

#endif // SUBDIVISIONROADGENERATIONSTRATEGY_H