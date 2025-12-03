#ifndef SUBDIVISIONROADGENERATIONSTRATEGY_H
#define SUBDIVISIONROADGENERATIONSTRATEGY_H

#include "AbstractRoadGenerationStrategy.h"
#include "../core/AbstractRoad.h"
#include <QRectF>
#include <QVector2D>
#include <QVector3D>
#include <vector>
#include <memory>
#include <set>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace City {

class SubdivisionRoadGenerationStrategy : public AbstractRoadGenerationStrategy {
public:
    SubdivisionRoadGenerationStrategy(
        float minBlockSize = 600.0f,  // Установлено 600 как значение по умолчанию
        int maxDepth = 10              // максимальная глубина рекурсии
    );

    std::vector<std::unique_ptr<AbstractRoad>> generate(
        float cityArea) override;

private:
    // Проверка углового ограничения между дорогами
    bool violatesAngleConstraint(
        const QVector3D& startPoint, 
        const QVector3D& endPoint,
        const std::vector<std::pair<QVector3D, QVector3D>>& roadSegments,
        float minAngle) const;
    // Основной метод рекурсивного разбиения кварталов
    void subdivide(const QRectF& rect, int depth, 
                  std::vector<std::unique_ptr<AbstractRoad>>& roads,
                  std::vector<QRectF>& blocks);
    
    // Генерация дорог внутри отдельного квартала
    std::vector<std::unique_ptr<AbstractRoad>> generateBlockRoads(
        const QRectF& blockRect) const;
    
    // Проверка валидности кандидата на дорогу (новый метод)
    bool isValidRoadCandidate(
        const QVector3D& start, 
        const QVector3D& end,
        const std::vector<std::unique_ptr<AbstractRoad>>& existingRoads,
        const QRectF& blockRect,
        const std::vector<QVector3D>& allPoints) const;
    
    // Проверка валидности соединения (старый метод, оставлен для совместимости)
    bool isConnectionValid(
        int node1, int node2, 
        const std::vector<QVector3D>& gridPoints,
        const std::vector<std::unique_ptr<AbstractRoad>>& existingRoads) const;
    
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