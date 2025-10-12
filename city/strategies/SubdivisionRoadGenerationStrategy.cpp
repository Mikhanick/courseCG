#include "SubdivisionRoadGenerationStrategy.h"
#include "../objects/ResidentialRoad.h"
#include <random>
#include <QVector2D>
#include <algorithm>
#include <set>

namespace City {

// Custom comparator for QPointF pairs to use in std::set
struct QPointFPairCompare {
    bool operator()(const std::pair<QPointF, QPointF>& a, const std::pair<QPointF, QPointF>& b) const {
        if (a.first.x() != b.first.x()) return a.first.x() < b.first.x();
        if (a.first.y() != b.first.y()) return a.first.y() < b.first.y();
        if (a.second.x() != b.second.x()) return a.second.x() < b.second.x();
        return a.second.y() < b.second.y();
    }
};

SubdivisionRoadGenerationStrategy::SubdivisionRoadGenerationStrategy(
    float minBlockSize, int maxDepth)
    : m_minBlockSize(minBlockSize)
    , m_maxDepth(maxDepth)
{
}

std::vector<std::unique_ptr<AbstractRoad>> SubdivisionRoadGenerationStrategy::generate(
    float cityArea, int totalPopulation)
{
    // Определяем границы города как квадрат
    float citySide = std::sqrt(cityArea);
    QRectF cityBounds(0, 0, citySide, citySide);

    // Начинаем с одного блока — всего города
    std::vector<Block> blocks = {{cityBounds, 0}};

    // Рекурсивное разбиение
    size_t i = 0;
    while (i < blocks.size()) {
        if (blocks[i].depth < m_maxDepth) {
            QRectF r = blocks[i].rect;
            if (r.width() > m_minBlockSize * 2 || r.height() > m_minBlockSize * 2) {
                subdivide(blocks, blocks[i]);
                blocks.erase(blocks.begin() + i); // заменяем блок его частями
                continue;
            }
        }
        ++i;
    }

    return blocksToRoads(blocks);
}

void SubdivisionRoadGenerationStrategy::subdivide(
    std::vector<Block>& blocks, const Block& block)
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<float> splitDist(0.3f, 0.7f);
    static std::uniform_int_distribution<int> axisDist(0, 1);

    QRectF r = block.rect;
    bool splitHorizontal = (r.width() >= r.height());
    if (r.width() < m_minBlockSize * 2 && r.height() >= m_minBlockSize * 2) {
        splitHorizontal = false;
    } else if (r.height() < m_minBlockSize * 2 && r.width() >= m_minBlockSize * 2) {
        splitHorizontal = true;
    } else {
        splitHorizontal = axisDist(gen) == 0;
    }

    float t = splitDist(gen);
    if (splitHorizontal) {
        float splitX = r.left() + r.width() * t;
        blocks.push_back({QRectF(r.left(), r.top(), splitX - r.left(), r.height()), block.depth + 1});
        blocks.push_back({QRectF(splitX, r.top(), r.right() - splitX, r.height()), block.depth + 1});
    } else {
        float splitY = r.top() + r.height() * t;
        blocks.push_back({QRectF(r.left(), r.top(), r.width(), splitY - r.top()), block.depth + 1});
        blocks.push_back({QRectF(r.left(), splitY, r.width(), r.bottom() - splitY), block.depth + 1});
    }
}

std::vector<std::unique_ptr<AbstractRoad>> SubdivisionRoadGenerationStrategy::blocksToRoads(
    const std::vector<Block>& blocks) const
{
    std::vector<std::unique_ptr<AbstractRoad>> roads;
    std::set<std::pair<QPointF, QPointF>, QPointFPairCompare> addedRoads; // избегаем дубликатов

    auto addUniqueRoad = [&](const QPointF& a, const QPointF& b) {
        QPointF p1 = a, p2 = b;
        if (p1.x() > p2.x() || (p1.x() == p2.x() && p1.y() > p2.y())) {
            std::swap(p1, p2);
        }
        if (addedRoads.insert({p1, p2}).second) {
            roads.push_back(std::make_unique<ResidentialRoad>(
                QVector3D(p1.x(), 0, p1.y()),
                QVector3D(p2.x(), 0, p2.y())
            ));
        }
    };

    // Для каждого блока добавляем его границы как дороги
    for (const auto& block : blocks) {
        QRectF r = block.rect;
        // Левая граница
        addUniqueRoad(QPointF(r.left(), r.top()), QPointF(r.left(), r.bottom()));
        // Правая граница
        addUniqueRoad(QPointF(r.right(), r.top()), QPointF(r.right(), r.bottom()));
        // Верхняя граница
        addUniqueRoad(QPointF(r.left(), r.top()), QPointF(r.right(), r.top()));
        // Нижняя граница
        addUniqueRoad(QPointF(r.left(), r.bottom()), QPointF(r.right(), r.bottom()));
    }

    return roads;
}

} // namespace City