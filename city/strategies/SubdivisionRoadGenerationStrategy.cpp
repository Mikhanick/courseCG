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
constexpr int MIN_ROAD_LENGTH_STEPS = 7;   // Minimum road length in grid steps
constexpr int MAX_ROAD_LENGTH_STEPS = 18;   // Maximum road length in grid steps
constexpr int MAX_ROADS_PER_BLOCK = 40;    // Maximum roads per block (W)
constexpr int MAX_RELOCATION_ATTEMPTS = 120; // Maximum relocation attempts for intersections (Q)
constexpr int EXCLUSION_RADIUS = 7; // Maximum relocation attempts for intersections (Q)
constexpr float GRID_STEP = 13.f;         // Grid step size
constexpr float BOUNDARY_BUFFER = 40; // Buffer from block boundary
constexpr int MAX_ROADS_PER_POINT = 5;     // Максимальное количество дорог в одной точке


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
    constexpr float CORNER_BUFFER = 3.0f * GRID_STEP;

    // Параметры проверки сторон (в блоках сетки) - настраиваемые константы
    constexpr float SEARCH_DISTANCE_PERPENDICULAR_BLOCKS = 2.5f; // N
    constexpr float OFFSET_FROM_ENDS_BLOCKS = 1.f;              // M
    const float searchDistance = SEARCH_DISTANCE_PERPENDICULAR_BLOCKS * GRID_STEP;
    const float offsetDistance = OFFSET_FROM_ENDS_BLOCKS * GRID_STEP;

    qDebug() << "\n=== ПАРАМЕТРЫ ЗАСТРОЙКИ ===";
    qDebug() << "  Перпендикулярное расстояние поиска (N):" << SEARCH_DISTANCE_PERPENDICULAR_BLOCKS 
             << "блоков →" << searchDistance << "единиц";
    qDebug() << "  Отступ от концов (M):" << OFFSET_FROM_ENDS_BLOCKS 
             << "блоков →" << offsetDistance << "единиц";
    qDebug() << "  Минимальная длина дороги:" << MIN_ROAD_LENGTH_STEPS * GRID_STEP << "единиц";
    qDebug() << "  Максимальная длина дороги:" << MAX_ROAD_LENGTH_STEPS * GRID_STEP << "единиц";

    std::vector<std::unique_ptr<AbstractRoad>> roads;
    auto findPointIndex = [EXCLUSION_EPSILON](const QVector3D& point, const std::vector<QVector3D>& points) -> int {
        for (size_t i = 0; i < points.size(); ++i) {
            if ((point - points[i]).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) {
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
    
    if (innerRect.width() < MIN_ROAD_LENGTH_STEPS * GRID_STEP || 
        innerRect.height() < MIN_ROAD_LENGTH_STEPS * GRID_STEP) {
        qDebug() << "!!! ВНУТРЕННЯЯ ОБЛАСТЬ СЛИШКОМ МАЛА ДЛЯ ГЕНЕРАЦИИ ДОРОГ";
        qDebug() << "    Ширина:" << innerRect.width() << ", Высота:" << innerRect.height();
        qDebug() << "    Минимально допустимый размер:" << MIN_ROAD_LENGTH_STEPS * GRID_STEP;
        return roads;
    }

    qDebug() << "\n=== СЕТКА ТОЧЕК ===";
    qDebug() << "Внутренняя область:" << innerRect.width() << "x" << innerRect.height();
    qDebug() << "Отступ от границ (BOUNDARY_BUFFER):" << BOUNDARY_BUFFER;
    qDebug() << "Шаг сетки (GRID_STEP):" << GRID_STEP;

    std::vector<QVector3D> gridPoints;
    int gridCount = 0;
    for (float x = innerRect.left(); x <= innerRect.right() + EXCLUSION_EPSILON; x += GRID_STEP) {
        for (float z = innerRect.top(); z <= innerRect.bottom() + EXCLUSION_EPSILON; z += GRID_STEP) {
            if (x <= innerRect.right() + EXCLUSION_EPSILON && z <= innerRect.bottom() + EXCLUSION_EPSILON) {
                gridPoints.push_back(QVector3D(x, 0, z));
                gridCount++;
            }
        }
    }
    
    qDebug() << "Создано внутренних точек сетки:" << gridPoints.size() << "(" << gridCount << ")";
    if (gridPoints.empty()) {
        qDebug() << "!!! СЕТКА ПУСТА - ЗАВЕРШЕНИЕ ГЕНЕРАЦИИ";
        return roads;
    }

    // 2. ГЕНЕРАЦИЯ ТОЧЕК НА ГРАНИЦАХ КВАРТАЛА
    qDebug() << "\n=== ГЕНЕРАЦИЯ ГРАНИЧНЫХ ТОЧЕК ===";
    std::vector<QVector3D> boundaryPoints;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    auto generateSidePoints = [&](float fixedCoord, float startCoord, float endCoord, bool isVertical, const QString& sideName) {
        std::uniform_int_distribution<> numPointsDist(1, 2);
        int numPoints = numPointsDist(gen);
        qDebug() << "  " << sideName << ": генерация" << numPoints << "точек";
        
        std::vector<float> coords;
        float sideLength = endCoord - startCoord;
        
        if (sideLength < 4.0f * GRID_STEP) {
            qDebug() << "    Сторона слишком короткая для генерации точек (длина:" << sideLength << ")";
            return;
        }
        
        for (int i = 0; i < numPoints; ++i) {
            float coord;
            bool valid;
            int attempts = 0;
            
            do {
                coord = startCoord + randomFloat(0.15f, 0.85f) * sideLength;
                valid = true;
                
                // Проверяем расстояние до других точек на этой стороне и до углов
                if (coord - startCoord < 2.0f * GRID_STEP || endCoord - coord < 2.0f * GRID_STEP) {
                    valid = false;
                }
                
                for (float existingCoord : coords) {
                    if (std::abs(coord - existingCoord) < 2.5f * GRID_STEP) {
                        valid = false;
                        break;
                    }
                }
                
                attempts++;
            } while (!valid && attempts < 20);
            
            if (valid) {
                coords.push_back(coord);
                qDebug() << "    " << sideName << ": добавлена точка на позиции" << coord 
                         << "(отступ от угла:" << coord - startCoord << ")";
            } else {
                qDebug() << "    " << sideName << ": не удалось сгенерировать точку после" << attempts << "попыток";
            }
        }
        
        // Создаем 3D точки
        for (float coord : coords) {
            if (isVertical) {
                boundaryPoints.push_back(QVector3D(fixedCoord, 0, coord));
            } else {
                boundaryPoints.push_back(QVector3D(coord, 0, fixedCoord));
            }
        }
    };
    
    // Генерация точек для всех четырех сторон
    generateSidePoints(blockRect.top(), 
                      blockRect.left(), 
                      blockRect.right(), 
                      false, "ВЕРХНЯЯ СТОРОНА");
    
    generateSidePoints(blockRect.bottom(), 
                      blockRect.left(), 
                      blockRect.right(), 
                      false, "НИЖНЯЯ СТОРОНА");
    
    generateSidePoints(blockRect.left(), 
                      blockRect.top(), 
                      blockRect.bottom(), 
                      true, "ЛЕВАЯ СТОРОНА");
    
    generateSidePoints(blockRect.right(), 
                      blockRect.top(), 
                      blockRect.bottom(), 
                      true, "ПРАВАЯ СТОРОНА");
    
    qDebug() << "\nСгенерировано граничных точек всего:" << boundaryPoints.size();
    for (size_t i = 0; i < boundaryPoints.size(); ++i) {
        qDebug() << "  Граничная точка" << i << ":" 
                 << "x=" << boundaryPoints[i].x() << ", z=" << boundaryPoints[i].z();
    }
    
    if (boundaryPoints.empty()) {
        qDebug() << "!!! НЕТ ГРАНИЧНЫХ ТОЧЕК ДЛЯ НАЧАЛА ГЕНЕРАЦИИ - ЗАВЕРШЕНИЕ";
        return roads;
    }

    // 3. Инициализация структур данных
    qDebug() << "\n=== ИНИЦИАЛИЗАЦИЯ СТРУКТУР ДАННЫХ ===";
    std::vector<QVector3D> visitedPoints;
    std::vector<std::pair<QVector3D, QVector3D>> roadSegments;
    std::vector<QRectF> exclusionZones;
    std::vector<QVector3D> allPoints;
    std::vector<int> roadCounts;

    const float exclusionSize = EXCLUSION_RADIUS * GRID_STEP;
    qDebug() << "Размер зоны запрета:" << exclusionSize << "единиц (радиус)";
    
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
            qDebug() << "  Добавлена новая точка в allPoints (всего:" << allPoints.size() << ")";
        }
    };
    
    // Добавляем все граничные точки в посещенные
    qDebug() << "\nДобавление граничных точек в посещенные:";
    for (const auto& point : boundaryPoints) {
        addVisitedPoint(point);
        qDebug() << "  Добавлена точка:" << point.x() << "," << point.z();
    }
    
    qDebug() << "\nНачальное состояние:";
    qDebug() << "  Посещенных точек:" << visitedPoints.size();
    qDebug() << "  Зон запрета:" << exclusionZones.size();
    qDebug() << "  Уникальных точек:" << allPoints.size();

    // 4. Вспомогательные функции
    // Проверка углов между дорогами (исправленная версия с встроенной нормализацией)
    auto violatesAngleConstraint = [EXCLUSION_EPSILON, this](
        const QVector3D& startPoint, 
        const QVector3D& endPoint,
        const std::vector<std::pair<QVector3D, QVector3D>>& roadSegments,
        float minAngle) -> bool
    {
        // Вспомогательная функция для нормализации направления
        auto normalizeDirection = [](const QVector3D& start, const QVector3D& end) -> QVector3D {
            QVector3D dir = end - start;
            float len = dir.length();
            if (len < 1e-6f) return QVector3D(0, 0, 0);
            return dir / len;
        };
        
        const QVector3D newDirStart = normalizeDirection(startPoint, endPoint);
        const QVector3D newDirEnd = -newDirStart; // Направление к конечной точке
        
        if (newDirStart.length() < 1e-6f) return false;
        
        // Проверка углов в начальной точке
        for (const auto& [existingStart, existingEnd] : roadSegments) {
            QVector3D existingDir;
            bool sharesStart = false;
            
            if ((existingStart - startPoint).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) {
                existingDir = normalizeDirection(existingStart, existingEnd);
                sharesStart = true;
            } else if ((existingEnd - startPoint).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) {
                existingDir = normalizeDirection(existingEnd, existingStart);
                sharesStart = true;
            }
            
            if (sharesStart && existingDir.length() > 1e-6f) {
                float angle = calculateAngleBetweenVectors(newDirStart, existingDir);
                if (angle < minAngle - 1e-5f) {
                    return true;
                }
            }
        }
        
        // Проверка углов в конечной точке
        for (const auto& [existingStart, existingEnd] : roadSegments) {
            QVector3D existingDir;
            bool sharesEnd = false;
            
            if ((existingStart - endPoint).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) {
                existingDir = normalizeDirection(existingStart, existingEnd);
                sharesEnd = true;
            } else if ((existingEnd - endPoint).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) {
                existingDir = normalizeDirection(existingEnd, existingStart);
                sharesEnd = true;
            }
            
            if (sharesEnd && existingDir.length() > 1e-6f) {
                float angle = calculateAngleBetweenVectors(newDirEnd, existingDir);
                if (angle < minAngle - 1e-5f) {
                    return true;
                }
            }
        }
        
        return false;
    };

    // 5. Генерация дорог от граничных точек внутрь квартала
    qDebug() << "\n=== ГЕНЕРАЦИЯ ДОРОГ ===";
    int consecutiveFails = 0;
    int totalAttempts = 0;
    int successfulRoads = 0;

    while (successfulRoads < MAX_ROADS_PER_BLOCK && !visitedPoints.empty() && 
           consecutiveFails < MAX_RELOCATION_ATTEMPTS) 
    {
        bool roadAdded = false;
        
        for (int attempt = 0; attempt < MAX_RELOCATION_ATTEMPTS && !visitedPoints.empty(); ++attempt) {
            totalAttempts++;
            
            // Выбор начальной точки (из посещенных)
            std::uniform_int_distribution<> visitedDist(0, static_cast<int>(visitedPoints.size()) - 1);
            int startIdx = visitedDist(gen);
            QVector3D startPoint = visitedPoints[startIdx];
            
            // Выбор конечной точки (из внутренней сетки)
            std::uniform_int_distribution<> gridDist(0, static_cast<int>(gridPoints.size()) - 1);
            int endIdx = gridDist(gen);
            QVector3D endPoint = gridPoints[endIdx];
            
            // Проверка расстояния
            float distance = (endPoint - startPoint).length();
            float minAllowedLength = MIN_ROAD_LENGTH_STEPS * GRID_STEP;
            float maxAllowedLength = MAX_ROAD_LENGTH_STEPS * GRID_STEP;
            
            if (distance < minAllowedLength) {
                continue;
            }
            
            if (distance > maxAllowedLength) {
                // Оптимизация: если точка слишком далеко, ищем ближайшую допустимую
                QVector3D dir = (endPoint - startPoint).normalized();
                endPoint = startPoint + dir * (maxAllowedLength * 0.9f);
            }
            
            // Проверка: является ли endPoint существующей конечной точкой?
            bool isExistingEndpoint = false;
            for (const auto& [rStart, rEnd] : roadSegments) {
                if ((endPoint - rStart).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON || 
                    (endPoint - rEnd).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) {
                    isExistingEndpoint = true;
                    break;
                }
            }
            
            // Проверка зоны запрета (если не существующая конечная точка)
            if (!isExistingEndpoint) {
                bool inExclusionZone = false;
                
                for (size_t zi = 0; zi < exclusionZones.size(); ++zi) {
                    const auto& zone = exclusionZones[zi];
                    if (endPoint.x() >= zone.left() && endPoint.x() <= zone.right() &&
                        endPoint.z() >= zone.top() && endPoint.z() <= zone.bottom()) {
                        inExclusionZone = true;
                        break;
                    }
                }
                
                if (inExclusionZone) continue;
            }
            
            // Проверка углов между дорогами
            if (violatesAngleConstraint(startPoint, endPoint, roadSegments, MIN_ANGLE_BETWEEN_ROADS)) {
                continue;
            }
            
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
            
            if (roadCounts[startPointIdx] >= MAX_ROADS_PER_POINT || 
                roadCounts[endPointIdx] >= MAX_ROADS_PER_POINT) {
                continue;
            }
            
            // Проверка пересечений (оптимизированная)
            bool intersects = false;
            for (size_t ri = 0; ri < roadSegments.size(); ++ri) {
                const auto& [rStart, rEnd] = roadSegments[ri];
                
                // Пропускаем дороги, которые начинаются или заканчиваются в этих же точках
                bool sharesStart = ((startPoint - rStart).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON || 
                                   (startPoint - rEnd).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON);
                bool sharesEnd = ((endPoint - rStart).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON || 
                                 (endPoint - rEnd).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON);
                
                if (sharesStart && sharesEnd) continue;
                
                if (doLinesIntersect(startPoint, endPoint, rStart, rEnd)) {
                    QVector3D intersection = findLineIntersection(startPoint, endPoint, rStart, rEnd);
                    
                    // Проверяем, не является ли пересечение конечной точкой существующей дороги
                    bool isEndpointIntersection = 
                        ((intersection - rStart).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON || 
                         (intersection - rEnd).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) &&
                        ((intersection - startPoint).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON || 
                         (intersection - endPoint).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON);
                    
                    if (!isEndpointIntersection) {
                        intersects = true;
                        break;
                    }
                }
            }
            if (intersects) continue;
            
            // Успешное добавление дороги
            roadSegments.emplace_back(startPoint, endPoint);
            roadCounts[startPointIdx]++;
            roadCounts[endPointIdx]++;
            
            // Добавляем конечную точку в посещенные
            bool endPointExists = false;
            for (const auto& vp : visitedPoints) {
                if ((endPoint - vp).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) {
                    endPointExists = true;
                    break;
                }
            }
            
            if (!endPointExists) {
                addVisitedPoint(endPoint);
            }
            
            successfulRoads++;
            roadAdded = true;
            consecutiveFails = 0;
            break;
        }
        
        if (!roadAdded) {
            consecutiveFails++;
            if (consecutiveFails >= MAX_RELOCATION_ATTEMPTS / 3 && visitedPoints.size() > boundaryPoints.size()) {
                // Удаляем последнюю добавленную точку для разблокировки
                if (visitedPoints.size() > boundaryPoints.size()) {
                    visitedPoints.pop_back();
                    if (!exclusionZones.empty()) exclusionZones.pop_back();
                    qDebug() << "  УДАЛЕНА ПОСЛЕДНЯЯ ПОСЕЩЕННАЯ ТОЧКА для разблокировки генерации";
                }
            }
        }
        
        if (totalAttempts > 5000) {
            qDebug() << "  ДОСТИГНУТ ЛИМИТ ПОПЫТОК (5000), ПРЕРЫВАНИЕ ЦИКЛА";
            break;
        }
    }

    qDebug() << "\n=== РЕЗУЛЬТАТЫ ГЕНЕРАЦИИ ДОРОГ ===";
    qDebug() << "Успешно создано дорог:" << successfulRoads;
    qDebug() << "Всего попыток:" << totalAttempts;
    qDebug() << "Последовательных неудач:" << consecutiveFails;
    qDebug() << "Посещенных точек:" << visitedPoints.size();
    qDebug() << "Всего сегментов:" << roadSegments.size();

    if (roadSegments.empty()) {
        qDebug() << "!!! НЕ СОЗДАНО НИ ОДНОЙ ВНУТРЕННЕЙ ДОРОГИ";
        return roads;
    }

    // 6. СОБИРАЕМ ВСЕ ДОРОГИ КВАРТАЛА ДЛЯ ПРОВЕРКИ СТОРОН
    qDebug() << "\n=== ПОДГОТОВКА К ПРОВЕРКЕ СТОРОН ЗАСТРОЙКИ ===";
    std::vector<std::pair<QVector3D, QVector3D>> allExistingRoads;
    
    // Добавляем границы квартала
    allExistingRoads.emplace_back(QVector3D(blockRect.left(), 0, blockRect.top()), 
                                  QVector3D(blockRect.right(), 0, blockRect.top()));
    allExistingRoads.emplace_back(QVector3D(blockRect.right(), 0, blockRect.top()), 
                                  QVector3D(blockRect.right(), 0, blockRect.bottom()));
    allExistingRoads.emplace_back(QVector3D(blockRect.right(), 0, blockRect.bottom()), 
                                  QVector3D(blockRect.left(), 0, blockRect.bottom()));
    allExistingRoads.emplace_back(QVector3D(blockRect.left(), 0, blockRect.bottom()), 
                                  QVector3D(blockRect.left(), 0, blockRect.top()));
    
    qDebug() << "Добавлены 4 границы квартала";
    
    // Добавляем внутренние дороги
    for (const auto& seg : roadSegments) {
        allExistingRoads.push_back(seg);
    }
    
    qDebug() << "Всего дорог для проверки:" << allExistingRoads.size() 
             << "(4 границы +" << roadSegments.size() << " внутренних)";

    // 7. ЛЯМБДА ДЛЯ ОПРЕДЕЛЕНИЯ СТОРОНЫ ЗАСТРОЙКИ (С УНИФИЦИРОВАННЫМ ENUM)
    auto determineBuildingSide = [this, searchDistance, offsetDistance, EXCLUSION_EPSILON]
        (const QVector3D& start, const QVector3D& end,
         const std::vector<std::pair<QVector3D, QVector3D>>& otherRoads) -> BuildingSide
    {
        const float roadLength = (end - start).length();
        
        // Проверка на вырожденную дорогу
        if (roadLength < 1e-3f) {
            qDebug() << "  ВЫРОЖДЕННАЯ ДОРОГА (длина = 0), сторона: NONE";
            return BuildingSide::NONE;
        }

        // Нормализованное направление дороги (от start к end)
        QVector3D dir = (end - start).normalized();
        
        // Проверка корректности направления
        if (dir.length() < 1e-6f) {
            qDebug() << "  НЕВОЗМОЖНО ОПРЕДЕЛИТЬ НАПРАВЛЕНИЕ ДОРОГИ, сторона: NONE";
            return BuildingSide::NONE;
        }

        // Перпендикулярные векторы (лево/право относительно направления движения)
        QVector3D perpLeft(-dir.z(), 0, dir.x()); // Поворот на 90 градусов против часовой стрелки
        float perpLength = perpLeft.length();
        
        if (perpLength < 1e-6f) {
            qDebug() << "  НЕВОЗМОЖНО ОПРЕДЕЛИТЬ ПЕРПЕНДИКУЛЯР, сторона: NONE";
            return BuildingSide::NONE;
        }
        
        perpLeft.normalize();
        QVector3D perpRight = -perpLeft; // Поворот на 90 градусов по часовой стрелке

        // Адаптивный отступ от концов дороги
        float effectiveOffset = offsetDistance;
        if (roadLength < 2 * offsetDistance) {
            effectiveOffset = roadLength * 0.2f; // 20% от длины для коротких дорог
            qDebug() << "  Короткая дорога (длина:" << roadLength 
                     << "), уменьшаем отступ до:" << effectiveOffset;
        }
        
        // Вычисление среднего сегмента для проверки
        QVector3D middleStart = start + dir * effectiveOffset;
        QVector3D middleEnd = end - dir * effectiveOffset;
        float middleLength = (middleEnd - middleStart).length();
        
        // Если средний сегмент слишком короткий, используем центральную точку
        if (middleLength < 1e-3f) {
            middleStart = (start + end) * 0.5f;
            middleEnd = middleStart;
            middleLength = 0.0f;
        }

        bool leftHasRoad = false;
        bool rightHasRoad = false;
        
        // Оптимизация: прерываем проверку, если обе стороны уже заняты
        bool bothSidesBlocked = false;
        
        if (middleLength > 1e-3f) {
            // Определяем количество точек выборки (от 1 до 5)
            int numSamples = 1;
            if (middleLength > 2 * GRID_STEP) numSamples = 2;
            if (middleLength > 4 * GRID_STEP) numSamples = 3;
            if (middleLength > 6 * GRID_STEP) numSamples = 4;
            if (middleLength > 8 * GRID_STEP) numSamples = 5;
            
            qDebug() << "  Проверка среднего сегмента (длина:" << middleLength 
                     << "), точек выборки:" << numSamples;
            
            for (int i = 0; i < numSamples && !bothSidesBlocked; ++i) {
                float t = (numSamples > 1) ? static_cast<float>(i) / (numSamples - 1) : 0.5f;
                QVector3D P = middleStart + (middleEnd - middleStart) * t;
                
                // Проверка левой стороны
                if (!leftHasRoad) {
                    QVector3D leftRayEnd = P + perpLeft * searchDistance;
                    
                    for (const auto& seg : otherRoads) {
                        if (this->doLinesIntersect(P, leftRayEnd, seg.first, seg.second)) {
                            leftHasRoad = true;
                            qDebug() << "    НАЙДЕНА дорога СЛЕВА в точке t=" << t;
                            break;
                        }
                    }
                }
                
                // Проверка правой стороны
                if (!rightHasRoad) {
                    QVector3D rightRayEnd = P + perpRight * searchDistance;
                    
                    for (const auto& seg : otherRoads) {
                        if (this->doLinesIntersect(P, rightRayEnd, seg.first, seg.second)) {
                            rightHasRoad = true;
                            qDebug() << "    НАЙДЕНА дорога СПРАВА в точке t=" << t;
                            break;
                        }
                    }
                }
                
                bothSidesBlocked = leftHasRoad && rightHasRoad;
            }
        } else {
            // Для очень коротких дорог проверяем только центральную точку
            QVector3D P = middleStart;
            
            // Проверка левой стороны
            QVector3D leftRayEnd = P + perpLeft * searchDistance;
            for (const auto& seg : otherRoads) {
                if (this->doLinesIntersect(P, leftRayEnd, seg.first, seg.second)) {
                    leftHasRoad = true;
                    qDebug() << "    НАЙДЕНА дорога СЛЕВА в центральной точке";
                    break;
                }
            }
            
            // Проверка правой стороны
            QVector3D rightRayEnd = P + perpRight * searchDistance;
            for (const auto& seg : otherRoads) {
                if (this->doLinesIntersect(P, rightRayEnd, seg.first, seg.second)) {
                    rightHasRoad = true;
                    qDebug() << "    НАЙДЕНА дорога СПРАВА в центральной точке";
                    break;
                }
            }
        }

        // ОПРЕДЕЛЕНИЕ СТОРОНЫ СОГЛАСНО УНИФИЦИРОВАННОМУ ENUM:
        BuildingSide side;
        if (!leftHasRoad && !rightHasRoad) {
            side = BuildingSide::BOTH;
            qDebug() << "  Результат: НЕТ ДОРОГ СЛЕВА И СПРАВА →" << buildingSideToString(side);
        } else if (!leftHasRoad) {
            side = BuildingSide::LEFT;
            qDebug() << "  Результат: НЕТ ДОРОГИ СЛЕВА →" << buildingSideToString(side);
        } else if (!rightHasRoad) {
            side = BuildingSide::RIGHT;
            qDebug() << "  Результат: НЕТ ДОРОГИ СПРАВА →" << buildingSideToString(side);
        } else {
            side = BuildingSide::NONE;
            qDebug() << "  Результат: ДОРОГИ С ОБЕИХ СТОРОН →" << buildingSideToString(side);
        }
        
        return side;
    };

    // 8. СОЗДАНИЕ ОБЪЕКТОВ ДОРОГ С ОПРЕДЕЛЕНИЕМ СТОРОНЫ ЗАСТРОЙКИ
    qDebug() << "\n=== СОЗДАНИЕ ОБЪЕКТОВ ДОРОГ ===";
    for (size_t i = 0; i < roadSegments.size(); ++i) {
        const auto& [start, end] = roadSegments[i];
        qDebug() << "\nОбработка дороги #" << i 
                 << " от (" << start.x() << "," << start.z() << ") до (" << end.x() << "," << end.z() << ")";
        qDebug() << "  Длина дороги:" << (end - start).length();

        // Формируем список дорог для проверки (без текущей)
        std::vector<std::pair<QVector3D, QVector3D>> otherRoads;
        for (const auto& seg : allExistingRoads) {
            bool isCurrent = false;
            
            // Точное сравнение с учетом обоих направлений
            if (((seg.first - start).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON && 
                 (seg.second - end).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON) ||
                ((seg.first - end).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON && 
                 (seg.second - start).lengthSquared() < EXCLUSION_EPSILON * EXCLUSION_EPSILON)) {
                isCurrent = true;
            }
            
            if (!isCurrent) {
                otherRoads.push_back(seg);
            }
        }
        
        qDebug() << "  Дорог для проверки сторон:" << otherRoads.size() 
                 << "(всего:" << allExistingRoads.size() << ", исключена текущая)";

        // Определяем сторону для зданий
        BuildingSide side = determineBuildingSide(start, end, otherRoads);

        // Создаём объект дороги
        auto road = std::make_unique<ResidentialRoad>(start, end, 6.0f);
        road->setBuildingSideFromEnum(side);
        roads.push_back(std::move(road));

        // Логирование
        int startIdx = findPointIndex(start, allPoints);
        int endIdx = findPointIndex(end, allPoints);
        int startCount = (startIdx != -1) ? roadCounts[startIdx] : 0;
        int endCount = (endIdx != -1) ? roadCounts[endIdx] : 0;
        
        qDebug() << "Дорога #" << i << ":" 
                 << "от (" << start.x() << "," << start.z() << ")[" << startCount << "] до ("
                 << end.x() << "," << end.z() << ")[" << endCount << "]"
                 << " длина:" << (end - start).length()
                 << " сторона:" << buildingSideToString(side);
    }
    
    qDebug() << "\n===== ЗАВЕРШЕНИЕ ГЕНЕРАЦИИ =====";
    qDebug() << "Итоговые результаты:";
    qDebug() << "  Всего дорог создано:" << roads.size() << "/" << MAX_ROADS_PER_BLOCK;
    qDebug() << "  Посещенных точек:" << visitedPoints.size();
    qDebug() << "  Граничных точек:" << boundaryPoints.size();
    qDebug() << "  Уникальных точек:" << allPoints.size();
    qDebug() << "  Всего попыток:" << totalAttempts;
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