#include "SubdivisionRoadGenerationStrategy.h"
#include "../objects/ResidentialRoad.h"
#include <random>
#include <QVector2D>
#include <algorithm>
#include <set>
#include <cmath>

namespace City {

void SubdivisionRoadGenerationStrategy::Block::generateInternalRoads(const std::vector<std::unique_ptr<AbstractRoad>>& externalRoads) {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);
    
    float blockWidth = static_cast<float>(rect.width());
    float blockHeight = static_cast<float>(rect.height());
    float blockLeft = static_cast<float>(rect.left());
    float blockTop = static_cast<float>(rect.top());
    float blockRight = static_cast<float>(rect.right());
    float blockBottom = static_cast<float>(rect.bottom());
    
    // Randomly determine building side
    std::uniform_int_distribution<int> sideDist(-1, 1);
    
    // Ensure at least one through road connects opposite sides of the block
    // Randomly choose whether to create a horizontal or vertical through road
    bool createHorizontalThroughRoad = uniformDist(gen) > 0.5f;
    
    if (createHorizontalThroughRoad) {
        // Create a horizontal through road (connects left and right sides)
        float roadY = blockTop + blockHeight * (0.3f + 0.4f * uniformDist(gen)); // Keep road in middle 40% of block
        auto throughRoad = std::make_unique<ResidentialRoad>(
            QVector3D(blockLeft + 10.0f, 0, roadY),
            QVector3D(blockRight - 10.0f, 0, roadY),
            4.0f
        );
        
        throughRoad->setBuildingSide(sideDist(gen));
        roads.push_back(std::move(throughRoad));
    } else {
        // Create a vertical through road (connects top and bottom sides)
        float roadX = blockLeft + blockWidth * (0.3f + 0.4f * uniformDist(gen)); // Keep road in middle 40% of block
        auto throughRoad = std::make_unique<ResidentialRoad>(
            QVector3D(roadX, 0, blockTop + 10.0f),
            QVector3D(roadX, 0, blockBottom - 10.0f),
            4.0f
        );
        
        throughRoad->setBuildingSide(sideDist(gen));
        roads.push_back(std::move(throughRoad));
    }
    
    // With some probability, add roads connecting adjacent sides
    if (uniformDist(gen) > 0.3f) {  // 70% chance to add adjacent connecting roads
        int numAdjacentRoads = 1 + static_cast<int>(uniformDist(gen) * 3);  // 1-4 adjacent roads
        
        for (int i = 0; i < numAdjacentRoads; ++i) {
            // Randomly choose type of adjacent connection
            int connectionType = static_cast<int>(uniformDist(gen) * 4);  // 0-3
            
            switch (connectionType) {
                case 0: {  // Left to top
                    float leftX = blockLeft + 10.0f + uniformDist(gen) * (blockWidth * 0.3f);
                    float topY = blockTop + 10.0f + uniformDist(gen) * (blockHeight * 0.3f);
                    
                    auto road = std::make_unique<ResidentialRoad>(
                        QVector3D(leftX, 0, blockTop + blockHeight * 0.5f),  // Start from middle of left side
                        QVector3D(blockLeft + blockWidth * 0.5f, 0, topY),    // End at middle of top side
                        4.0f
                    );
                    
                    road->setBuildingSide(sideDist(gen));
                    roads.push_back(std::move(road));
                    break;
                }
                case 1: {  // Right to top
                    float rightX = blockRight - 10.0f - uniformDist(gen) * (blockWidth * 0.3f);
                    float topY = blockTop + 10.0f + uniformDist(gen) * (blockHeight * 0.3f);
                    
                    auto road = std::make_unique<ResidentialRoad>(
                        QVector3D(rightX, 0, blockTop + blockHeight * 0.5f),  // Start from middle of right side
                        QVector3D(blockLeft + blockWidth * 0.5f, 0, topY),    // End at middle of top side
                        4.0f
                    );
                    
                    road->setBuildingSide(sideDist(gen));
                    roads.push_back(std::move(road));
                    break;
                }
                case 2: {  // Right to bottom
                    float rightX = blockRight - 10.0f - uniformDist(gen) * (blockWidth * 0.3f);
                    float bottomY = blockBottom - 10.0f - uniformDist(gen) * (blockHeight * 0.3f);
                    
                    auto road = std::make_unique<ResidentialRoad>(
                        QVector3D(rightX, 0, blockTop + blockHeight * 0.5f),  // Start from middle of right side
                        QVector3D(blockLeft + blockWidth * 0.5f, 0, bottomY), // End at middle of bottom side
                        4.0f
                    );
                    
                    road->setBuildingSide(sideDist(gen));
                    roads.push_back(std::move(road));
                    break;
                }
                case 3: {  // Left to bottom
                    float leftX = blockLeft + 10.0f + uniformDist(gen) * (blockWidth * 0.3f);
                    float bottomY = blockBottom - 10.0f - uniformDist(gen) * (blockHeight * 0.3f);
                    
                    auto road = std::make_unique<ResidentialRoad>(
                        QVector3D(leftX, 0, blockTop + blockHeight * 0.5f),  // Start from middle of left side
                        QVector3D(blockLeft + blockWidth * 0.5f, 0, bottomY), // End at middle of bottom side
                        4.0f
                    );
                    
                    road->setBuildingSide(sideDist(gen));
                    roads.push_back(std::move(road));
                    break;
                }
            }
        }
    }
    
    // Create branching roads with maximum length of 3 segments
    int numBranches = static_cast<int>(uniformDist(gen) * 5);  // 0-4 branching roads
    
    for (int i = 0; i < numBranches; ++i) {
        // Start from an existing road or from a random position
        QVector3D startPos;
        if (!roads.empty() && uniformDist(gen) > 0.3f) {
            // Start from an existing road
            int roadIdx = static_cast<int>(uniformDist(gen) * roads.size());
            auto& existingRoad = roads[roadIdx];
            float t = uniformDist(gen);  // Position along the road (0 to 1)
            QVector3D roadStart = existingRoad->getStart();
            QVector3D roadEnd = existingRoad->getEnd();
            startPos = roadStart + (roadEnd - roadStart) * t;
        } else {
            // Start from a random position within the block
            startPos = QVector3D(
                blockLeft + 10.0f + uniformDist(gen) * (blockWidth - 20.0f),
                0,
                blockTop + 10.0f + uniformDist(gen) * (blockHeight - 20.0f)
            );
        }
        
        // Create a branching road with up to 3 segments
        int numSegments = 1 + static_cast<int>(uniformDist(gen) * 3);  // 1-3 segments
        QVector3D currentPos = startPos;
        
        for (int seg = 0; seg < numSegments; ++seg) {
            // Calculate a random direction and length
            float angle = uniformDist(gen) * 2.0f * M_PI;  // Random angle in radians
            float maxSegmentLength = 30.0f;  // Max length of each segment
            
            // Calculate end position of this segment
            float endX = currentPos.x() + cos(angle) * maxSegmentLength;
            float endZ = currentPos.z() + sin(angle) * maxSegmentLength;
            
            // Keep within block boundaries with some margin
            endX = std::max(blockLeft + 5.0f, std::min(blockRight - 5.0f, endX));
            endZ = std::max(blockTop + 5.0f, std::min(blockBottom - 5.0f, endZ));
            
            QVector3D endPos(endX, 0, endZ);
            
            // Check for intersection with existing roads before adding
            bool intersects = false;
            for (const auto& existingRoad : roads) {
                QVector3D existingStart = existingRoad->getStart();
                QVector3D existingEnd = existingRoad->getEnd();
                
                // Simple intersection check (not perfect but sufficient for now)
                if (doLinesIntersect(currentPos, endPos, existingStart, existingEnd)) {
                    // If intersection occurs, stop the branch at the intersection point
                    QVector3D intersectionPoint = findLineIntersection(currentPos, endPos, existingStart, existingEnd);
                    if (intersectionPoint != QVector3D(0, 0, 0)) {
                        endPos = intersectionPoint;
                        intersects = true;
                        break;
                    }
                }
            }
            
            // Create the road segment
            auto branchRoad = std::make_unique<ResidentialRoad>(currentPos, endPos, 3.0f);
            branchRoad->setBuildingSide(sideDist(gen));
            roads.push_back(std::move(branchRoad));
            
            currentPos = endPos;
            
            // If there was an intersection, stop creating more segments for this branch
            if (intersects) {
                break;
            }
        }
    }
}

// Helper function to check if two lines intersect
bool SubdivisionRoadGenerationStrategy::doLinesIntersect(const QVector3D& line1Start, const QVector3D& line1End,
                                                        const QVector3D& line2Start, const QVector3D& line2End) {
    float x1 = line1Start.x(), y1 = line1Start.z();
    float x2 = line1End.x(), y2 = line1End.z();
    float x3 = line2Start.x(), y3 = line2Start.z();
    float x4 = line2End.x(), y4 = line2End.z();
    
    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < 1e-6f) return false;  // Lines are parallel
    
    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;
    
    // Check if intersection point is within both line segments
    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

// Helper function to find intersection point of two lines
QVector3D SubdivisionRoadGenerationStrategy::findLineIntersection(const QVector3D& line1Start, const QVector3D& line1End,
                                                                  const QVector3D& line2Start, const QVector3D& line2End) {
    float x1 = line1Start.x(), y1 = line1Start.z();
    float x2 = line1End.x(), y2 = line1End.z();
    float x3 = line2Start.x(), y3 = line2Start.z();
    float x4 = line2End.x(), y4 = line2End.z();
    
    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < 1e-6f) return QVector3D(0, 0, 0);  // Lines are parallel
    
    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    
    // Calculate intersection point using line1
    float intersectX = x1 + t * (x2 - x1);
    float intersectY = y1 + t * (y2 - y1);
    
    return QVector3D(intersectX, 0, intersectY);
}

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
    std::vector<Block> blocks;
    blocks.push_back({cityBounds, 0, {}});

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

    // Генерируем дороги из блоков (external roads)
    std::vector<std::unique_ptr<AbstractRoad>> roads = blocksToRoads(blocks);
    
    // Collect all internal roads from blocks
    for (const auto& block : blocks) {
        for (const auto& road : block.roads) {
            // Clone the road using the prototype pattern
            roads.push_back(road->clone());
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
    
    // RULE 1: For large blocks (min side > 500) - use only rectangular subdivisions
    if (minSide > 500.0f) {
        bool splitHorizontal = (r.width() >= r.height());
        if (r.width() < 400.0f && r.height() >= 300.0f) {
            splitHorizontal = false;
        } else if (r.height() < 300.0f && r.width() >= 500.0f) {
            splitHorizontal = true;
        } else {
            splitHorizontal = axisDist(gen) == 0;
        }

        float t = splitDist(gen);
        if (splitHorizontal) {
            float splitX = r.left() + r.width() * t;
            blocks.push_back({QRectF(r.left(), r.top(), splitX - r.left(), r.height()), block.depth + 1, {}});
            blocks.push_back({QRectF(splitX, r.top(), r.right() - splitX, r.height()), block.depth + 1, {}});
        } else {
            float splitY = r.top() + r.height() * t;
            blocks.push_back({QRectF(r.left(), r.top(), r.width(), splitY - r.top()), block.depth + 1, {}});
            blocks.push_back({QRectF(r.left(), splitY, r.width(), r.bottom() - splitY), block.depth + 1, {}});
        }
    }
    // RULE 2: For medium blocks (400 <= min side <= 700) - use alternative subdivisions with internal roads
    else {
        // Create a new block with the rectangle and depth
        Block newBlock = {r, block.depth + 1, {}};
        
        // Create external roads that bound the block - these will be passed to internal road generation
        std::vector<std::unique_ptr<AbstractRoad>> externalRoads;
        
        // Create roads for each side of the block
        externalRoads.push_back(std::make_unique<ResidentialRoad>(
            QVector3D(r.left(), 0, r.top()), QVector3D(r.right(), 0, r.top()), 20.0f)); // Top
        externalRoads.push_back(std::make_unique<ResidentialRoad>(
            QVector3D(r.right(), 0, r.top()), QVector3D(r.right(), 0, r.bottom()), 20.0f)); // Right
        externalRoads.push_back(std::make_unique<ResidentialRoad>(
            QVector3D(r.right(), 0, r.bottom()), QVector3D(r.left(), 0, r.bottom()), 20.0f)); // Bottom
        externalRoads.push_back(std::make_unique<ResidentialRoad>(
            QVector3D(r.left(), 0, r.bottom()), QVector3D(r.left(), 0, r.top()), 20.0f)); // Left
        
        // Generate internal roads within the block using the external roads as constraints
        newBlock.generateInternalRoads(externalRoads);
        
        // Mark the block as fully generated by adding it to the blocks list
        blocks.push_back(std::move(newBlock));
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