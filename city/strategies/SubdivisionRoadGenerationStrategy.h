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
        int maxDepth = 10              // максимальная глубина рекурсии
    );

    std::vector<std::unique_ptr<AbstractRoad>> generate(
        float cityArea, int totalPopulation) override;

private:
    class Block {
    public:
        QRectF rect;
        int depth;
        std::vector<std::unique_ptr<AbstractRoad>> roads;
        
        // Constructor
        Block(const QRectF& r, int d, std::vector<std::unique_ptr<AbstractRoad>> rds = {})
            : rect(r), depth(d), roads(std::move(rds)) {}
        
        // Method to generate internal roads within the block
        void generateInternalRoads(const std::vector<std::unique_ptr<AbstractRoad>>& externalRoads);
    };

    void subdivide(std::vector<Block>& blocks, const Block& block);
    std::vector<std::unique_ptr<AbstractRoad>> blocksToRoads(const std::vector<Block>& blocks) const;
    
    // Helper functions for intersection detection
    static bool doLinesIntersect(const QVector3D& line1Start, const QVector3D& line1End,
                                 const QVector3D& line2Start, const QVector3D& line2End);
    static QVector3D findLineIntersection(const QVector3D& line1Start, const QVector3D& line1End,
                                          const QVector3D& line2Start, const QVector3D& line2End);

    float m_minBlockSize;
    int m_maxDepth;
};

} // namespace City

#endif // SUBDIVISIONROADGENERATIONSTRATEGY_H