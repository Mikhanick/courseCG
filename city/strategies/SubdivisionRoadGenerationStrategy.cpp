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
            // Изменяем условие: блоки делятся до тех пор, пока не станут меньше 500 на 300
            if (r.width() > 500.0f || r.height() > 300.0f) {
                subdivide(blocks, blocks[i]);
                blocks.erase(blocks.begin() + i); // заменяем блок его частями
                continue;
            }
        }
        ++i;
    }

    // Генерируем дороги из блоков
    std::vector<std::unique_ptr<AbstractRoad>> roads = blocksToRoads(blocks);
    
    // Добавляем внутренние дороги с поворотами и перекрестками для финальных блоков
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    for (const auto& block : blocks) {
        // All blocks in the final list are "final" blocks, so add internal roads to all of them
        // Определяем размеры блока
        float blockWidth = static_cast<float>(block.rect.width());
        float blockHeight = static_cast<float>(block.rect.height());
        
        // Количество внутренних дорог зависит от размера блока
        int numInternalRoads = std::max(1, static_cast<int>(std::min(blockWidth, blockHeight) / 100.0f));
        
        for (int i = 0; i < numInternalRoads; ++i) {
            // Создаем дороги с поворотами внутри блока
            std::uniform_real_distribution<float> blockXDist(block.rect.left(), block.rect.right());
            std::uniform_real_distribution<float> blockZDist(block.rect.top(), block.rect.bottom());
            float startX = blockXDist(gen);
            float startZ = blockZDist(gen);
            
            // Ограничиваем максимальную длину дороги в пределах блока
            float maxSegmentLength = std::min(blockWidth, blockHeight) * 0.4f;
            
            // Создаем дорогу с поворотами (ломаную линию)
            int numSegments = 2 + (gen() % 3); // 2-4 сегмента
            QVector3D currentPos(startX, 0, startZ);
            
            for (int seg = 0; seg < numSegments; ++seg) {
                // Случайное направление и длина для следующего сегмента
                float angle = (gen() % 360) * static_cast<float>(M_PI) / 180.0f; // случайный угол
                float length = 20.0f + (gen() % static_cast<int>(maxSegmentLength - 20.0f)); // от 20 до maxSegmentLength
                
                float nextX = std::max(static_cast<float>(block.rect.left()) + 10.0f, 
                                      std::min(static_cast<float>(block.rect.right()) - 10.0f, 
                                              currentPos.x() + static_cast<float>(cos(angle)) * length));
                float nextZ = std::max(static_cast<float>(block.rect.top()) + 10.0f, 
                                      std::min(static_cast<float>(block.rect.bottom()) - 10.0f, 
                                              currentPos.z() + static_cast<float>(sin(angle)) * length));
                
                QVector3D nextPos(nextX, 0, nextZ);
                
                // Добавляем сегмент дороги
                roads.push_back(std::make_unique<ResidentialRoad>(currentPos, nextPos, 4.0f));
                
                currentPos = nextPos;
            }
        }
        
        // Также добавляем перекрестки в центре некоторых блоков
        if (gen() % 3 == 0) { // примерно 1 из 3 блоков
            float centerX = static_cast<float>(block.rect.left()) + blockWidth * 0.5f;
            float centerY = static_cast<float>(block.rect.top()) + blockHeight * 0.5f;
            
            // Горизонтальная дорога перекрестка
            float crossStartX = std::max(static_cast<float>(block.rect.left()) + 10.0f, 
                                        centerX - blockWidth * 0.3f);
            float crossEndX = std::min(static_cast<float>(block.rect.right()) - 10.0f, 
                                      centerX + blockWidth * 0.3f);
            
            // Вертикальная дорога перекрестка
            float crossStartZ = std::max(static_cast<float>(block.rect.top()) + 10.0f, 
                                        centerY - blockHeight * 0.3f);
            float crossEndZ = std::min(static_cast<float>(block.rect.bottom()) - 10.0f, 
                                      centerY + blockHeight * 0.3f);
            
            roads.push_back(std::make_unique<ResidentialRoad>(
                QVector3D(crossStartX, 0, centerY),
                QVector3D(crossEndX, 0, centerY),
                5.0f
            ));
            
            roads.push_back(std::make_unique<ResidentialRoad>(
                QVector3D(centerX, 0, crossStartZ),
                QVector3D(centerX, 0, crossEndZ),
                5.0f
            ));
        }
    }
    
    return roads;
}

void SubdivisionRoadGenerationStrategy::subdivide(
    std::vector<Block>& blocks, const Block& block)
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<float> splitDist(0.3f, 0.7f);
    static std::uniform_int_distribution<int> axisDist(0, 1);

    QRectF r = block.rect;
    
    // Determine minimum side length to decide subdivision strategy
    float minSide = std::min(r.width(), r.height());
    
    // RULE 1: For large blocks (min side > 700) - use only rectangular subdivisions
    if (minSide > 700.0f) {
        bool splitHorizontal = (r.width() >= r.height());
        if (r.width() < 500.0f && r.height() >= 300.0f) {
            splitHorizontal = false;
        } else if (r.height() < 300.0f && r.width() >= 500.0f) {
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
    // RULE 2: For medium blocks (400 <= min side <= 700) - use alternative subdivisions with internal roads
    else if (minSide >= 400.0f && minSide <= 700.0f) {
        static std::uniform_int_distribution<int> subdivisionTypeDist(1, 4); // 4 alternative subdivision types
        int subdivisionType = subdivisionTypeDist(gen);
        
        switch(subdivisionType) {
            case 1: // T-shaped subdivision (3 blocks)
            {
                bool splitHorizontal = axisDist(gen) == 0;
                float t = splitDist(gen);
                
                if (splitHorizontal) {
                    float splitX = r.left() + r.width() * t;
                    float subSplitY = r.top() + r.height() * 0.5f;
                    blocks.push_back({QRectF(r.left(), r.top(), splitX - r.left(), r.height()), block.depth + 1});
                    blocks.push_back({QRectF(splitX, r.top(), r.right() - splitX, subSplitY - r.top()), block.depth + 1});
                    blocks.push_back({QRectF(splitX, subSplitY, r.right() - splitX, r.bottom() - subSplitY), block.depth + 1});
                } else {
                    float splitY = r.top() + r.height() * t;
                    float subSplitX = r.left() + r.width() * 0.5f;
                    blocks.push_back({QRectF(r.left(), r.top(), r.width(), splitY - r.top()), block.depth + 1});
                    blocks.push_back({QRectF(r.left(), splitY, subSplitX - r.left(), r.bottom() - splitY), block.depth + 1});
                    blocks.push_back({QRectF(subSplitX, splitY, r.right() - subSplitX, r.bottom() - splitY), block.depth + 1});
                }
                break;
            }
            case 2: // Cross-shaped subdivision (4 blocks)
            {
                float centerX = r.left() + r.width() * 0.5f;
                float centerY = r.top() + r.height() * 0.5f;
                
                blocks.push_back({QRectF(r.left(), r.top(), centerX - r.left(), centerY - r.top()), block.depth + 1});
                blocks.push_back({QRectF(centerX, r.top(), r.right() - centerX, centerY - r.top()), block.depth + 1});
                blocks.push_back({QRectF(r.left(), centerY, centerX - r.left(), r.bottom() - centerY), block.depth + 1});
                blocks.push_back({QRectF(centerX, centerY, r.right() - centerX, r.bottom() - centerY), block.depth + 1});
                break;
            }
            case 3: // L-shaped subdivision
            {
                bool leftTop = axisDist(gen) == 0;
                float t = splitDist(gen);
                
                if (leftTop) {
                    float splitX = r.left() + r.width() * t;
                    float splitY = r.top() + r.height() * t;
                    
                    blocks.push_back({QRectF(splitX, r.top(), r.right() - splitX, splitY - r.top()), block.depth + 1});
                    blocks.push_back({QRectF(r.left(), r.top(), splitX - r.left(), r.bottom() - r.top()), block.depth + 1});
                } else {
                    float splitX = r.left() + r.width() * t;
                    float splitY = r.top() + r.height() * t;
                    
                    blocks.push_back({QRectF(r.left(), splitY, splitX - r.left(), r.bottom() - splitY), block.depth + 1});
                    blocks.push_back({QRectF(splitX, r.top(), r.right() - splitX, r.bottom() - r.top()), block.depth + 1});
                }
                break;
            }
            case 4: // Corner subdivision
            {
                bool cutTopLeft = axisDist(gen) == 0;
                float t = splitDist(gen);
                
                if (cutTopLeft) {
                    float cutX = r.left() + r.width() * t;
                    float cutY = r.top() + r.height() * t;
                    
                    blocks.push_back({QRectF(cutX, r.top(), r.right() - cutX, r.height()), block.depth + 1});
                    blocks.push_back({QRectF(r.left(), cutY, cutX - r.left(), r.bottom() - cutY), block.depth + 1});
                    blocks.push_back({QRectF(r.left(), r.top(), cutX - r.left(), cutY - r.top()), block.depth + 1});
                } else {
                    float cutX = r.left() + r.width() * (1.0f - t);
                    float cutY = r.top() + r.height() * (1.0f - t);
                    
                    blocks.push_back({QRectF(r.left(), r.top(), cutX - r.left(), r.height()), block.depth + 1});
                    blocks.push_back({QRectF(cutX, r.top(), r.right() - cutX, cutY - r.top()), block.depth + 1});
                    blocks.push_back({QRectF(cutX, cutY, r.right() - cutX, r.bottom() - cutY), block.depth + 1});
                }
                break;
            }
        }
    }
    // For small blocks (min side < 400) - these are final blocks, add them as is
    else {
        blocks.push_back({r, block.depth + 1}); // Add the block as final, don't subdivide further
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