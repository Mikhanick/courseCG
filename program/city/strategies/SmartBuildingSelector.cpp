#include "SmartBuildingSelector.h"
#include "../loaders/ModelLoader.h"
#include <algorithm>
#include <cmath>
#include <QDir>
#include <QDebug>
#include <stdexcept>
#include <QtMath>
#include <random>  // For std::mt19937
#include "../../GlobalRandom.h"  // Include global random functionality

using namespace City;

SmartBuildingSelector::SmartBuildingSelector(const QString& modelsDir) {
    availableModels = ModelLoader::loadModelsFromDirectory(modelsDir);

    if (availableModels.empty()) {
        qFatal("CRITICAL ERROR: No building models loaded from %s. Application cannot continue.",
               qPrintable(modelsDir));
        throw std::runtime_error("No building models available");
    }

    qDebug() << "Successfully loaded" << availableModels.size()
             << "building models from" << modelsDir;

    // Вывод размеров всех загруженных моделей
    qDebug() << "=== DIMENSIONS OF ALL LOADED BUILDING MODELS ===";
    for (const auto& model : availableModels) {
        QSizeF groundDims = getBaseDimensions(model.groundFloor);
        float height = getSectionHeight(model.groundFloor) +
                      (model.floorCount - 1) * getSectionHeight(model.typicalFloor) +
                      getSectionHeight(model.roof);

        qDebug() << "Model:" << model.name
                 << "| Ground dimensions:" << groundDims.width() << "x" << groundDims.height()
                 << "| Height:" << height
                 << "| Min/Max width:" << model.minWidth << "/" << model.maxWidth
                 << "| Min/Max depth:" << model.minDepth << "/" << model.maxDepth
                 << "| Floor count:" << model.floorCount
                 << "| Fixed scale:" << model.fixedScale;
    }

    randomEngine.seed(randomSeed);
    updateRandomGenerators(randomSeed);  // Ensure global generator is initialized too
}

void SmartBuildingSelector::setSeed(unsigned int seed) {
    randomSeed = seed;  // Update the global seed
    updateRandomGenerators(seed);  // Update global generator
    randomEngine.seed(randomSeed);  // Update local engine
}

QSizeF SmartBuildingSelector::getBaseDimensions(const FloorSection& section) const {
    if (section.vertices.empty()) return QSizeF(1.0f, 1.0f);

    float minX = section.vertices[0].x();
    float maxX = section.vertices[0].x();
    float minZ = section.vertices[0].z();
    float maxZ = section.vertices[0].z();

    for (const auto& v : section.vertices) {
        minX = std::min(minX, v.x());
        maxX = std::max(maxX, v.x());
        minZ = std::min(minZ, v.z());
        maxZ = std::max(maxZ, v.z());
    }

    return QSizeF(std::max(0.1f, maxX - minX), std::max(0.1f, maxZ - minZ));
}

float SmartBuildingSelector::getSectionHeight(const FloorSection& section) const {
    if (section.vertices.empty()) return 3.0f;

    float minY = section.vertices[0].y();
    float maxY = section.vertices[0].y();

    for (const auto& v : section.vertices) {
        minY = std::min(minY, v.y());
        maxY = std::max(maxY, v.y());
    }

    return std::max(0.1f, maxY - minY);
}

BuildingModel SmartBuildingSelector::chooseBestModel(const QSizeF& availableSize) const {
    std::vector<BuildingModel> suitableModels;

    // Сначала ищем модели с фиксированным масштабом, которые помещаются
    for (const auto& model : availableModels) {
        if (model.fixedScale) {
            // Для фиксированных моделей проверяем, вписываются ли доступные размеры в допустимый диапазон модели
            // т.е. доступная ширина должна быть >= minWidth и <= maxWidth модели
            // и доступная глубина должна быть >= minDepth и <= maxDepth модели
            if (availableSize.width() >= model.minWidth &&
                availableSize.width() <= model.maxWidth &&
                availableSize.height() >= model.minDepth &&
                availableSize.height() <= model.maxDepth) {
                    suitableModels.push_back(model);
            }
        }
    }

    // Если нет подходящих фиксированных моделей - ищем обычные
    if (suitableModels.empty()) {
        for (const auto& model : availableModels) {
            if (!model.fixedScale &&
                availableSize.width() >= model.minWidth &&
                availableSize.width() <= model.maxWidth &&
                availableSize.height() >= model.minDepth &&
                availableSize.height() <= model.maxDepth) {
                suitableModels.push_back(model);
            }
        }
    }

    // Если все еще нет подходящих - ищем хотя бы частично подходящие модели
    if (suitableModels.empty()) {
        // Проверяем, есть ли модели, которые хотя бы минимальные размеры имеют
        for (const auto& model : availableModels) {
            if (availableSize.width() >= model.minWidth &&
                availableSize.height() >= model.minDepth) {
                suitableModels.push_back(model);
            }
        }
    }

    if (suitableModels.empty()) {
        // Если нет подходящих моделей, возвращаем первую доступную
        if (!availableModels.empty()) {
            std::uniform_int_distribution<size_t> dist(0, availableModels.size() - 1);
            return availableModels[dist(randomEngine)];
        }
        throw std::runtime_error("No building models available");
    }

    // Случайный выбор из подходящих моделей (вместо выбора по наибольшей площади)
    std::uniform_int_distribution<size_t> dist(0, suitableModels.size() - 1);
    return suitableModels[dist(randomEngine)];
}GraphicObject SmartBuildingSelector::buildFromModel(
    const BuildingModel& model,
    const QSizeF& availableSize) const
{
    GraphicObject result;
    const float margin = 0.f;
    float startX = margin;
    float startZ = margin;

    // Получаем базовые размеры модели
    QSizeF groundDims = getBaseDimensions(model.groundFloor);

    // Определяем масштаб в зависимости от настройки fixedScale
    float scaleX, scaleZ;

    if (model.fixedScale) {
        // Полностью фиксированный масштаб — без изменений
        scaleX = 1.0f;
        scaleZ = 1.0f;
    } else {
        // Только масштабирование по основанию, без ограничений
        scaleX = availableSize.width() / groundDims.width();
        scaleZ = availableSize.height() / groundDims.height();

        // УБРАНО: ограничения через min/max размеры
        // Теперь масштаб может быть любым — только для заполнения участка
    }

    int vertexOffset = 0;
    float currentY = 0.0f;

    // 1. СТРОИМ ПЕРВЫЙ ЭТАЖ — БЕЗ ИЗМЕНЕНИЯ ВЫСОТЫ
    for (const auto& v : model.groundFloor.vertices) {
        float x = startX + v.x() * scaleX;
        float y = v.y(); // ← ИСПОЛЬЗУЕМ ОРИГИНАЛЬНУЮ ВЫСОТУ ИЗ ФАЙЛА
        float z = startZ + v.z() * scaleZ;
        result.AddPoint(QVector3D(x, y, z));
    }

    for (const auto& face : model.groundFloor.faces) {
        result.AddFace(
            vertexOffset + face.index0,
            vertexOffset + face.index1,
            vertexOffset + face.index2,
            face.color
        );
    }
    vertexOffset += model.groundFloor.vertices.size();

    // Определяем высоту первого этажа по его вершинам
    float groundFloorHeight = getSectionHeight(model.groundFloor);
    currentY = groundFloorHeight;

    // 2. СТРОИМ ТИПОВЫЕ ЭТАЖИ — БЕЗ МАСШТАБИРОВАНИЯ ВЫСОТЫ
    float typicalFloorHeight = getSectionHeight(model.typicalFloor);

    for (int floor = 0; floor < model.floorCount - 1; ++floor) {
        float yBase = currentY + floor * typicalFloorHeight;

        for (const auto& v : model.typicalFloor.vertices) {
            float x = startX + v.x() * scaleX;
            float y = yBase + v.y(); // ← ОРИГИНАЛЬНАЯ ВЫСОТА ИЗ ФАЙЛА
            float z = startZ + v.z() * scaleZ;
            result.AddPoint(QVector3D(x, y, z));
        }

        for (const auto& face : model.typicalFloor.faces) {
            result.AddFace(
                vertexOffset + face.index0,
                vertexOffset + face.index1,
                vertexOffset + face.index2,
                face.color
            );
        }
        vertexOffset += model.typicalFloor.vertices.size();
    }

    // 3. СТРОИМ КРЫШУ — БЕЗ МАСШТАБИРОВАНИЯ ВЫСОТЫ
    float roofBaseY = currentY + (model.floorCount - 1) * typicalFloorHeight;

    for (const auto& v : model.roof.vertices) {
        float x = startX + v.x() * scaleX;
        float y = roofBaseY + v.y(); // ← ОРИГИНАЛЬНАЯ ВЫСОТА ИЗ ФАЙЛА
        float z = startZ + v.z() * scaleZ;
        result.AddPoint(QVector3D(x, y, z));
    }

    for (const auto& face : model.roof.faces) {
        result.AddFace(
            vertexOffset + face.index0,
            vertexOffset + face.index1,
            vertexOffset + face.index2,
            face.color
        );
    }

    result.ComputeFaceNormals();
    return result;
}

GraphicObject SmartBuildingSelector::select(const QRectF& availableArea) {
    // Рассчитываем доступное пространство с отступами
    const float margin = 5.0f;
    float availableWidth = std::max(25.0f, static_cast<float>(availableArea.width()) - 2 * margin);
    float availableDepth = std::max(25.0f, static_cast<float>(availableArea.height()) - 2 * margin);

    QSizeF availableSize(availableWidth, availableDepth);

    // Вывод информации о размере участка
    qDebug() << "\n=== PLOT SIZE FOR BUILDING SELECTION ===";
    qDebug() << "Original available area:" << availableArea.width() << "x" << availableArea.height();
    qDebug() << "Available size after margin (" << margin << "):" << availableSize.width() << "x" << availableSize.height();

    BuildingModel model = chooseBestModel(availableSize);
    return buildFromModel(model, availableSize);
}