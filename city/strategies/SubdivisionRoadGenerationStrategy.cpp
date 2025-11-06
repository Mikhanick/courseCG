#include "SubdivisionRoadGenerationStrategy.h"
#include "../objects/ResidentialRoad.h"
#include "../objects/BlockSeparatorRoad.h"
#include <random>
#include <QVector2D>
#include <algorithm>
#include <set>
#include <cmath>
#include <QVector3D>
#include <limits>
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace City {

// Constants for road generation
constexpr int MIN_ROAD_LENGTH_STEPS = 6;   // Minimum road length in grid steps
constexpr int MAX_ROAD_LENGTH_STEPS = 18;   // Maximum road length in grid steps
constexpr int MAX_ROADS_PER_BLOCK = 40;    // Maximum roads per block (W)
constexpr int MAX_RELOCATION_ATTEMPTS = 120; // Maximum relocation attempts for intersections (Q)
constexpr int EXCLUSION_RADIUS = 8; // Maximum relocation attempts for intersections (Q)
constexpr float GRID_STEP = 10.0f;         // Grid step size
constexpr float BOUNDARY_BUFFER = 30; // Buffer from block boundary
constexpr int MAX_ROADS_PER_POINT = 4;     // Максимальное количество дорог в одной точке


// Helper function to generate random float between min and max
float randomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

// Helper function to calculate angle between two vectors
float calculateAngleBetweenVectors(const QVector3D& v1, const QVector3D& v2) {
    QVector3D a = v1.normalized();
    QVector3D b = v2.normalized();
    float dot = QVector3D::dotProduct(a, b);
    return std::acos(std::clamp(dot, -1.0f, 1.0f));
}

// Helper function to check if point is near boundary
bool isNearBoundary(const QVector3D& point, const QRectF& blockRect) {
    return (point.x() <= blockRect.left() + 2 * GRID_STEP ||
            point.x() >= blockRect.right() - 2 * GRID_STEP ||
            point.z() <= blockRect.top() + 2 * GRID_STEP ||
            point.z() >= blockRect.bottom() - 2 * GRID_STEP);
}

// Constructor
SubdivisionRoadGenerationStrategy::SubdivisionRoadGenerationStrategy(
    float minBlockSize, int maxDepth)
    : m_minBlockSize(minBlockSize)
    , m_maxDepth(maxDepth)
{
}

// Main generation method
std::vector<std::unique_ptr<AbstractRoad>> SubdivisionRoadGenerationStrategy::generate(
    float cityArea)
{
    // Определяем границы города как квартал
    float citySide = std::sqrt(cityArea);
    QRectF cityBounds(0, 0, citySide, citySide);

    // Создаем начальные дороги для границ города
    std::vector<std::unique_ptr<AbstractRoad>> roads;
    
    // Добавляем границы города как основные дороги
    roads.push_back(std::make_unique<BlockSeparatorRoad>(
        QVector3D(cityBounds.left(), 0, cityBounds.top()), 
        QVector3D(cityBounds.right(), 0, cityBounds.top()), 20.0f)); // Top
    roads.push_back(std::make_unique<BlockSeparatorRoad>(
        QVector3D(cityBounds.right(), 0, cityBounds.top()), 
        QVector3D(cityBounds.right(), 0, cityBounds.bottom()), 20.0f)); // Right
    roads.push_back(std::make_unique<BlockSeparatorRoad>(
        QVector3D(cityBounds.right(), 0, cityBounds.bottom()), 
        QVector3D(cityBounds.left(), 0, cityBounds.bottom()), 20.0f)); // Bottom
    roads.push_back(std::make_unique<BlockSeparatorRoad>(
        QVector3D(cityBounds.left(), 0, cityBounds.bottom()), 
        QVector3D(cityBounds.left(), 0, cityBounds.top()), 20.0f)); // Left

    // Собираем все кварталы и добавляем разделительные дороги
    std::vector<QRectF> allBlocks;
    subdivide(cityBounds, 0, roads, allBlocks);
    
    // Генерируем внутренние дороги для каждого квартала
    for (const QRectF& block : allBlocks) {
        auto blockRoads = generateBlockRoads(block);
        for (auto& road : blockRoads) {
            roads.push_back(std::move(road));
        }
    }
    
    return roads;
}

void SubdivisionRoadGenerationStrategy::subdivide(const QRectF& rect, int depth, 
                                                 std::vector<std::unique_ptr<AbstractRoad>>& roads,
                                                 std::vector<QRectF>& blocks) {
    // Проверяем условия остановки рекурсии
    float width = rect.width();
    float height = rect.height();
    float longestSide = std::max(width, height);
    
    // Используем параметр из конструктора
    if (longestSide < m_minBlockSize || depth >= m_maxDepth) {
        blocks.push_back(rect);
        return;
    }
    
    // Определяем направление разбиения
    bool splitVertical = (width > height);
    float splitRatio = randomFloat(0.3f, 0.7f);
    
    if (splitVertical) {
        // Вертикальное разбиение
        float splitX = rect.left() + width * splitRatio;
        QRectF leftBlock(rect.left(), rect.top(), splitX - rect.left(), height);
        QRectF rightBlock(splitX, rect.top(), rect.right() - splitX, height);
        
        // Добавляем разделительную дорогу
        roads.push_back(std::make_unique<BlockSeparatorRoad>(
            QVector3D(splitX, 0, rect.top()),
            QVector3D(splitX, 0, rect.bottom()),
            20.0f
        ));
        
        subdivide(leftBlock, depth + 1, roads, blocks);
        subdivide(rightBlock, depth + 1, roads, blocks);
    } else {
        // Горизонтальное разбиение
        float splitY = rect.top() + height * splitRatio;
        QRectF topBlock(rect.left(), rect.top(), width, splitY - rect.top());
        QRectF bottomBlock(rect.left(), splitY, width, rect.bottom() - splitY);
        
        // Добавляем разделительную дорогу
        roads.push_back(std::make_unique<BlockSeparatorRoad>(
            QVector3D(rect.left(), 0, splitY),
            QVector3D(rect.right(), 0, splitY),
            20.0f
        ));
        
        subdivide(topBlock, depth + 1, roads, blocks);
        subdivide(bottomBlock, depth + 1, roads, blocks);
    }
}

#include <QDebug>
std::vector<std::unique_ptr<AbstractRoad>> SubdivisionRoadGenerationStrategy::generateBlockRoads(
    const QRectF& blockRect) const
{
    qDebug() << "\n===== НАЧАЛО ГЕНЕРАЦИИ ДОРОГ ДЛЯ КВАРТАЛА =====";
    qDebug() << "Размер квартала:" << blockRect.width() << "x" << blockRect.height();
    constexpr float EXCLUSION_EPSILON = 1e-5f;
    constexpr float MIN_ANGLE_BETWEEN_ROADS = M_PI / 3.5f; 
    constexpr float CORNER_BUFFER = 3.0f * GRID_STEP; // Минимальный отступ от углов квартала

    // Параметры для определения стороны застройки (в блоках сетки)
    constexpr float SEARCH_DISTANCE_PERPENDICULAR_BLOCKS = 3.0f; // N — расстояние перпендикулярно дороге
    constexpr float OFFSET_FROM_ENDS_BLOCKS = 1.0f;              // M — отступ от концов
    const float searchDistance = SEARCH_DISTANCE_PERPENDICULAR_BLOCKS * GRID_STEP;
    const float offsetDistance = OFFSET_FROM_ENDS_BLOCKS * GRID_STEP;

    qDebug() << "Параметры застройки:";
    qDebug() << "  N (перпендикулярная зона поиска):" << SEARCH_DISTANCE_PERPENDICULAR_BLOCKS << "блоков →" << searchDistance << "ед";
    qDebug() << "  M (зона отступов от концов):" << OFFSET_FROM_ENDS_BLOCKS << "блоков →" << offsetDistance << "ед";

    std::vector<std::unique_ptr<AbstractRoad>> roads;
    auto findPointIndex = [EXCLUSION_EPSILON](const QVector3D& point, const std::vector<QVector3D>& points) -> int {
        for (size_t i = 0; i < points.size(); ++i) {
            if ((point - points[i]).length() < EXCLUSION_EPSILON) {
                return static_cast<int>(i);
            }
        }
        return -1;
    };

    // 1. Создаем сетку точек для внутренних дорог
    QRectF innerRect(
        blockRect.left() + BOUNDARY_BUFFER,
        blockRect.top() + BOUNDARY_BUFFER,
        blockRect.width() - 2 * BOUNDARY_BUFFER,
        blockRect.height() - 2 * BOUNDARY_BUFFER
    );

    qDebug() << "Внутренняя область:" << innerRect.width() << "x" << innerRect.height();
    qDebug() << "Отступ от границ (BOUNDARY_BUFFER):" << BOUNDARY_BUFFER;
    qDebug() << "Шаг сетки (GRID_STEP):" << GRID_STEP;
    qDebug() << "Мин/макс длина дороги (в шагах):" << MIN_ROAD_LENGTH_STEPS << "/" << MAX_ROAD_LENGTH_STEPS;
    qDebug() << "Радиус запретной зоны (EXCLUSION_RADIUS):" << EXCLUSION_RADIUS << "шагов сетки";
    qDebug() << "Макс дорог в точке (MAX_ROADS_PER_POINT):" << MAX_ROADS_PER_POINT;
    qDebug() << "!!! АКТИВИРОВАНА РАСШИРЕННАЯ ПРОВЕРКА УГЛОВ (начало + конец)";
    qDebug() << "!!! НОВАЯ ЛОГИКА: 1-2 точки генерации на каждой стороне квартала (отступ от углов:" << CORNER_BUFFER << ")";

    std::vector<QVector3D> gridPoints;
    for (float x = innerRect.left(); x <= innerRect.right() + EXCLUSION_EPSILON; x += GRID_STEP) {
        for (float z = innerRect.top(); z <= innerRect.bottom() + EXCLUSION_EPSILON; z += GRID_STEP) {
            if (x <= innerRect.right() + EXCLUSION_EPSILON && z <= innerRect.bottom() + EXCLUSION_EPSILON) {
                gridPoints.push_back(QVector3D(x, 0, z));
            }
        }
    }

    qDebug() << "Создано внутренних точек сетки:" << gridPoints.size();
    if (gridPoints.empty()) {
        qDebug() << "!!! СЕТКА ПУСТА - ЗАВЕРШЕНИЕ ГЕНЕРАЦИИ";
        return roads;
    }

    // 2. ГЕНЕРАЦИЯ ТОЧЕК НА ГРАНИЦАХ КВАРТАЛА (1-2 точки на каждую сторону)
    qDebug() << "\n--- ГЕНЕРАЦИЯ ГРАНИЧНЫХ ТОЧЕК ---";
    std::vector<QVector3D> boundaryPoints;
    std::random_device rd;
    std::mt19937 gen(rd());

    // Вспомогательная лямбда для генерации точек на стороне с мин. дистанцией
    auto generateSidePoints = [&](float fixedCoord, float startCoord, float endCoord, bool isVertical) {
        std::uniform_int_distribution<> numPointsDist(1, 2);
        int numPoints = numPointsDist(gen);
        qDebug() << "  Генерация" << numPoints << "точек на стороне";
        
        std::vector<float> coords;
        float sideLength = endCoord - startCoord;

        for (int i = 0; i < numPoints; ++i) {
            float coord;
            bool valid;
            int attempts = 0;

            do {
                coord = startCoord + randomFloat(0.1f, 0.9f) * sideLength;
                valid = true;
                for (float existingCoord : coords) {
                    if (std::abs(coord - existingCoord) < 2.0f * GRID_STEP) {
                        valid = false;
                        break;
                    }
                }
                attempts++;
            } while (!valid && attempts < 10);

            if (valid) {
                coords.push_back(coord);
                qDebug() << "    Сгенерирована точка на позиции:" << coord;
            }
        }

        for (float coord : coords) {
            if (isVertical) {
                boundaryPoints.push_back(QVector3D(fixedCoord, 0, coord));
            } else {
                boundaryPoints.push_back(QVector3D(coord, 0, fixedCoord));
            }
        }
    };

    // Верхняя сторона (y = blockRect.top())
    qDebug() << "\n  === ВЕРХНЯЯ СТОРОНА ===";
    generateSidePoints(blockRect.top(), 
                       blockRect.left() + CORNER_BUFFER, 
                       blockRect.right() - CORNER_BUFFER, false);

    // Нижняя сторона (y = blockRect.bottom())
    qDebug() << "\n  === НИЖНЯЯ СТОРОНА ===";
    generateSidePoints(blockRect.bottom(), 
                       blockRect.left() + CORNER_BUFFER, 
                       blockRect.right() - CORNER_BUFFER, false);

    // Левая сторона (x = blockRect.left())
    qDebug() << "\n  === ЛЕВАЯ СТОРОНА ===";
    generateSidePoints(blockRect.left(), 
                       blockRect.top() + CORNER_BUFFER, 
                       blockRect.bottom() - CORNER_BUFFER, true);

    // Правая сторона (x = blockRect.right())
    qDebug() << "\n  === ПРАВАЯ СТОРОНА ===";
    generateSidePoints(blockRect.right(), 
                       blockRect.top() + CORNER_BUFFER, 
                       blockRect.bottom() - CORNER_BUFFER, true);

    qDebug() << "\nСгенерировано граничных точек всего:" << boundaryPoints.size();
    for (size_t i = 0; i < boundaryPoints.size(); ++i) {
        qDebug() << "  Граничная точка" << i << ":" << boundaryPoints[i].x() << "," << boundaryPoints[i].z();
    }

    // 3. Инициализация структур данных
    std::vector<QVector3D> visitedPoints;
    std::vector<std::pair<QVector3D, QVector3D>> roadSegments;
    std::vector<QRectF> exclusionZones;
    std::vector<QVector3D> allPoints;
    std::vector<int> roadCounts;

    const float exclusionSize = EXCLUSION_RADIUS * GRID_STEP;
    qDebug() << "!!! ФАКТИЧЕСКИЙ РАЗМЕР ЗОНЫ ЗАПРЕТА:" << exclusionSize << "единиц (радиус)";

    auto addVisitedPoint = [&](const QVector3D& point) {
        visitedPoints.push_back(point);
        exclusionZones.emplace_back(
            point.x() - exclusionSize,
            point.z() - exclusionSize,
            exclusionSize * 2,
            exclusionSize * 2
        );
        int idx = findPointIndex(point, allPoints);
        if (idx == -1) {
            allPoints.push_back(point);
            roadCounts.push_back(0);
        }
    };

    // Добавляем граничные точки
    qDebug() << "\n--- ИНИЦИАЛИЗАЦИЯ ГРАНИЧНЫХ ТОЧЕК ---";
    for (const auto& point : boundaryPoints) {
        addVisitedPoint(point);
    }

    qDebug() << "\nНачальное состояние:";
    qDebug() << "  Посещенных точек (граничные):" << visitedPoints.size();
    qDebug() << "  Зон запрета:" << exclusionZones.size();
    qDebug() << "  Уникальных точек для счетчиков:" << allPoints.size();

    // Вспомогательная функция для проверки углов
    auto violatesAngleConstraint = [EXCLUSION_EPSILON, this](
        const QVector3D& startPoint, 
        const QVector3D& endPoint,
        const std::vector<std::pair<QVector3D, QVector3D>>& roadSegments,
        float minAngle) -> bool
    {
        bool violationFound = false;
        QVector3D newDirectionFromStart = (endPoint - startPoint).normalized();

        // Проверка в начальной точке
        for (const auto& [existingStart, existingEnd] : roadSegments) {
            bool sharesStartPoint = false;
            QVector3D existingDirection;

            if ((existingStart - startPoint).length() < EXCLUSION_EPSILON) {
                sharesStartPoint = true;
                existingDirection = (existingEnd - existingStart).normalized();
            }
            else if ((existingEnd - startPoint).length() < EXCLUSION_EPSILON) {
                sharesStartPoint = true;
                existingDirection = (existingStart - existingEnd).normalized();
            }

            if (sharesStartPoint) {
                float angle = calculateAngleBetweenVectors(newDirectionFromStart, existingDirection);
                if (angle < minAngle) {
                    violationFound = true;
                }
            }
        }

        // Проверка в конечной точке
        QVector3D newDirectionToEnd = (startPoint - endPoint).normalized();

        for (const auto& [existingStart, existingEnd] : roadSegments) {
            bool sharesEndPoint = false;
            QVector3D existingDirection;

            if ((existingStart - endPoint).length() < EXCLUSION_EPSILON) {
                sharesEndPoint = true;
                existingDirection = (existingEnd - existingStart).normalized();
            }
            else if ((existingEnd - endPoint).length() < EXCLUSION_EPSILON) {
                sharesEndPoint = true;
                existingDirection = (existingStart - existingEnd).normalized();
            }

            if (sharesEndPoint) {
                float angle = calculateAngleBetweenVectors(newDirectionToEnd, existingDirection);
                if (angle < minAngle) {
                    violationFound = true;
                }
            }
        }
        return violationFound;
    };

    // 4. Генерация дорог от граничных точек внутрь квартала
    qDebug() << "\n--- ГЕНЕРАЦИЯ ДОРОГ ОТ ГРАНИЦ ВНУТРЬ ---";
    int consecutiveFails = 0;
    int totalAttempts = 0;

    while (roads.size() < MAX_ROADS_PER_BLOCK && !visitedPoints.empty() && consecutiveFails < MAX_RELOCATION_ATTEMPTS) {
        bool roadAdded = false;

        for (int attempt = 0; attempt < MAX_RELOCATION_ATTEMPTS && !visitedPoints.empty(); ++attempt) {
            totalAttempts++;

            // Выбор начальной точки (из посещённых)
            std::uniform_int_distribution<> visitedDist(0, visitedPoints.size() - 1);
            int startIdx = visitedDist(gen);
            QVector3D startPoint = visitedPoints[startIdx];

            // Выбор конечной точки (из внутренней сетки)
            std::uniform_int_distribution<> gridDist(0, gridPoints.size() - 1);
            int endIdx = gridDist(gen);
            QVector3D endPoint = gridPoints[endIdx];

            // Проверка длины
            float distance = (endPoint - startPoint).length();
            float minAllowedLength = MIN_ROAD_LENGTH_STEPS * GRID_STEP;
            float maxAllowedLength = MAX_ROAD_LENGTH_STEPS * GRID_STEP;

            if (distance < minAllowedLength || distance > maxAllowedLength) continue;

            // Проверка: является ли endPoint существующей конечной точкой?
            bool isExistingEndpoint = false;
            for (const auto& [rStart, rEnd] : roadSegments) {
                if ((endPoint - rStart).length() < EXCLUSION_EPSILON || 
                    (endPoint - rEnd).length() < EXCLUSION_EPSILON) {
                    isExistingEndpoint = true;
                    break;
                }
            }

            // Проверка зоны запрета (если не существующая конечная точка)
            if (!isExistingEndpoint) {
                bool inExclusionZone = false;
                for (const auto& zone : exclusionZones) {
                    if (endPoint.x() >= zone.left() - EXCLUSION_EPSILON && endPoint.x() <= zone.right() + EXCLUSION_EPSILON &&
                        endPoint.z() >= zone.top() - EXCLUSION_EPSILON && endPoint.z() <= zone.bottom() + EXCLUSION_EPSILON) {
                        inExclusionZone = true;
                        break;
                    }
                }
                if (inExclusionZone) continue;
            }

            // Проверка углов
            if (violatesAngleConstraint(startPoint, endPoint, roadSegments, MIN_ANGLE_BETWEEN_ROADS)) continue;

            // Проверка количества дорог в точках
            int startPointIdx = findPointIndex(startPoint, allPoints);
            int endPointIdx = findPointIndex(endPoint, allPoints);

            if (startPointIdx == -1) {
                allPoints.push_back(startPoint);
                roadCounts.push_back(0);
                startPointIdx = static_cast<int>(allPoints.size() - 1);
            }
            if (endPointIdx == -1) {
                allPoints.push_back(endPoint);
                roadCounts.push_back(0);
                endPointIdx = static_cast<int>(allPoints.size() - 1);
            }

            bool wouldExceedStart = (roadCounts[startPointIdx] + 1) > MAX_ROADS_PER_POINT;
            bool wouldExceedEnd = (roadCounts[endPointIdx] + 1) > MAX_ROADS_PER_POINT;
            if (wouldExceedStart || wouldExceedEnd) continue;

            // Проверка пересечений
            bool intersects = false;
            for (size_t ri = 0; ri < roadSegments.size(); ++ri) {
                const auto& [rStart, rEnd] = roadSegments[ri];

                bool sharesStart = ((startPoint - rStart).length() < EXCLUSION_EPSILON || 
                                   (startPoint - rEnd).length() < EXCLUSION_EPSILON);
                bool sharesEnd = ((endPoint - rStart).length() < EXCLUSION_EPSILON || 
                                 (endPoint - rEnd).length() < EXCLUSION_EPSILON);

                if (sharesStart && sharesEnd) continue;

                if (doLinesIntersect(startPoint, endPoint, rStart, rEnd)) {
                    QVector3D intersection = findLineIntersection(startPoint, endPoint, rStart, rEnd);
                    bool isEndpointIntersection = 
                        ((intersection - rStart).length() < EXCLUSION_EPSILON || 
                         (intersection - rEnd).length() < EXCLUSION_EPSILON) &&
                        ((intersection - startPoint).length() < EXCLUSION_EPSILON || 
                         (intersection - endPoint).length() < EXCLUSION_EPSILON);

                    if (!isEndpointIntersection) {
                        intersects = true;
                        break;
                    }
                }
            }
            if (intersects) continue;

            // Успешное добавление
            roadSegments.emplace_back(startPoint, endPoint);
            roadCounts[startPointIdx]++;
            roadCounts[endPointIdx]++;

            // Добавляем конечную точку в посещённые
            bool endPointExists = false;
            for (const auto& vp : visitedPoints) {
                if ((endPoint - vp).length() < EXCLUSION_EPSILON) {
                    endPointExists = true;
                    break;
                }
            }
            if (!endPointExists) {
                addVisitedPoint(endPoint);
            }

            roadAdded = true;
            consecutiveFails = 0;
            break;
        }

        if (!roadAdded) {
            consecutiveFails++;
            if (consecutiveFails >= MAX_RELOCATION_ATTEMPTS / 2 && visitedPoints.size() > boundaryPoints.size() * 2) {
                if (!visitedPoints.empty()) {
                    visitedPoints.pop_back();
                    if (!exclusionZones.empty()) exclusionZones.pop_back();
                }
            }
        }

        if (totalAttempts > 10000) break;
    }

    // 5. Собираем ВСЕ дороги в квартале для проверки сторон: границы + внутренние
    std::vector<std::pair<QVector3D, QVector3D>> allExistingRoads;
    // Границы квартала
    allExistingRoads.emplace_back(QVector3D(blockRect.left(), 0, blockRect.top()), 
                                  QVector3D(blockRect.right(), 0, blockRect.top()));
    allExistingRoads.emplace_back(QVector3D(blockRect.right(), 0, blockRect.top()), 
                                  QVector3D(blockRect.right(), 0, blockRect.bottom()));
    allExistingRoads.emplace_back(QVector3D(blockRect.right(), 0, blockRect.bottom()), 
                                  QVector3D(blockRect.left(), 0, blockRect.bottom()));
    allExistingRoads.emplace_back(QVector3D(blockRect.left(), 0, blockRect.bottom()), 
                                  QVector3D(blockRect.left(), 0, blockRect.top()));

    // Добавляем внутренние дороги
    for (const auto& seg : roadSegments) {
        allExistingRoads.push_back(seg);
    }

    qDebug() << "\n--- ОПРЕДЕЛЕНИЕ СТОРОН ЗАСТРОЙКИ ---";
    qDebug() << "Всего дорог для проверки:" << allExistingRoads.size() 
             << "(4 границы +" << roadSegments.size() << " внутренних)";

    // Вспомогательная лямбда: определяет BuildingSide для одной дороги
    auto determineBuildingSideForRoad = [this, searchDistance, offsetDistance, EXCLUSION_EPSILON]
        (const QVector3D& start, const QVector3D& end,
         const std::vector<std::pair<QVector3D, QVector3D>>& otherRoads) -> BuildingSide
    {
        const float roadLength = (end - start).length();
        if (roadLength < 1e-6f) return BuildingSide::NONE;

        const QVector3D dir = (end - start).normalized();
        bool leftHasRoad = false;
        bool rightHasRoad = false;

        // Адаптивный отступ для коротких дорог
        float effectiveOffset = offsetDistance;
        if (roadLength < 2 * offsetDistance) {
            effectiveOffset = std::max(0.0f, roadLength * 0.3f);
        }

        const QVector3D midStart = start + dir * effectiveOffset;
        const QVector3D midEnd = end - dir * effectiveOffset;
        const float midLength = (midEnd - midStart).length();

        if (midLength > 1e-6f) {
            // Перпендикуляры (в 2D: dir = (dx, dz) → perp = (-dz, dx))
            QVector3D perpLeft(-dir.z(), 0, dir.x());
            const float perpLen = perpLeft.length();
            if (perpLen < 1e-6f) return BuildingSide::NONE;
            perpLeft.normalize();
            const QVector3D perpRight = -perpLeft;

            // Выборка: равномерные точки (минимум 1, максимум 5)
            const int numSamples = std::max(1, std::min(5, static_cast<int>(midLength / GRID_STEP) + 1));
            for (int i = 0; i < numSamples; ++i) {
                const float t = (numSamples == 1) ? 0.5f : static_cast<float>(i) / (numSamples - 1);
                const QVector3D P = midStart + (midEnd - midStart) * t;

                // Проверка левой стороны
                if (!leftHasRoad) {
                    const QVector3D leftRayEnd = P + perpLeft * searchDistance;
                    for (const auto& seg : otherRoads) {
                        if (this->doLinesIntersect(P, leftRayEnd, seg.first, seg.second)) {
                            leftHasRoad = true;
                            break;
                        }
                    }
                }

                // Проверка правой стороны
                if (!rightHasRoad) {
                    const QVector3D rightRayEnd = P + perpRight * searchDistance;
                    for (const auto& seg : otherRoads) {
                        if (this->doLinesIntersect(P, rightRayEnd, seg.first, seg.second)) {
                            rightHasRoad = true;
                            break;
                        }
                    }
                }

                if (leftHasRoad && rightHasRoad) break;
            }
        }

        if (!leftHasRoad && !rightHasRoad) return BuildingSide::BOTH;
        if (!leftHasRoad) return BuildingSide::LEFT;
        if (!rightHasRoad) return BuildingSide::RIGHT;
        return BuildingSide::NONE;
    };

    // 6. Создание объектов дорог с установкой стороны застройки
    for (size_t i = 0; i < roadSegments.size(); ++i) {
        const auto& [start, end] = roadSegments[i];

        // Формируем список дорог БЕЗ текущей
        std::vector<std::pair<QVector3D, QVector3D>> otherRoads;
        for (const auto& seg : allExistingRoads) {
            // Точное сравнение начала и конца (с учётом направления)
            bool isCurrent =
                ((start - seg.first).length() < EXCLUSION_EPSILON && (end - seg.second).length() < EXCLUSION_EPSILON) ||
                ((start - seg.second).length() < EXCLUSION_EPSILON && (end - seg.first).length() < EXCLUSION_EPSILON);
            if (!isCurrent) {
                otherRoads.push_back(seg);
            }
        }

        BuildingSide side = determineBuildingSideForRoad(start, end, otherRoads);
        auto road = std::make_unique<ResidentialRoad>(start, end, 6.0f);
        road->setBuildingSideFromEnum(side);
        roads.push_back(std::move(road));

        // Отладка
        int startIdx = findPointIndex(start, allPoints);
        int endIdx = findPointIndex(end, allPoints);
        int sc = (startIdx != -1) ? roadCounts[startIdx] : 0;
        int ec = (endIdx != -1) ? roadCounts[endIdx] : 0;
        qDebug() << "Дорога #" << i << " (сторона:" << static_cast<int>(side) << "):"
                 << "от (" << start.x() << "," << start.z() << ")[" << sc << "] до ("
                 << end.x() << "," << end.z() << ")[" << ec << "]";
    }

    qDebug() << "\n===== ЗАВЕРШЕНИЕ ГЕНЕРАЦИИ =====";
    qDebug() << "Итоговые результаты:";
    qDebug() << "  Всего дорог создано:" << roads.size() << "/" << MAX_ROADS_PER_BLOCK;
    qDebug() << "  Посещенных точек:" << visitedPoints.size();
    qDebug() << "  Граничных точек:" << boundaryPoints.size();
    qDebug() << "  Уникальных точек:" << allPoints.size();
    qDebug() << "  Попыток:" << totalAttempts;
    qDebug() << "  Последовательных неудач:" << consecutiveFails;

    return roads;
}

// Helper function to check if two lines intersect
bool SubdivisionRoadGenerationStrategy::doLinesIntersect(const QVector3D& line1Start, const QVector3D& line1End,
                                                        const QVector3D& line2Start, const QVector3D& line2End) {
    float x1 = line1Start.x(), y1 = line1Start.z();
    float x2 = line1End.x(), y2 = line1End.z();
    float x3 = line2Start.x(), y3 = line2Start.z();
    float x4 = line2End.x(), y4 = line2End.z();
    
    // Calculate denominators
    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < 1e-6f) return false;  // Lines are parallel
    
    // Calculate numerators
    float num1 = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4);
    float num2 = (x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3);
    
    // Calculate parameters
    float t = num1 / denom;
    float u = -num2 / denom;
    
    // Check if intersection is within both segments
    return (t >= 0.0f - 1e-6f && t <= 1.0f + 1e-6f && u >= 0.0f - 1e-6f && u <= 1.0f + 1e-6f);
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

} // namespace City