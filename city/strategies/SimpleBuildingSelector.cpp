#include "SimpleBuildingSelector.h"
#include <QColor>
#include <algorithm>
#include <random>

namespace City {

GraphicObject SimpleBuildingSelector::select(
    const QRectF& availableArea)
{
    // Учитываем отступ 5 единиц со всех сторон
    float availableWidth = std::max(0.0f, static_cast<float>(availableArea.height()) - 10.0f);  // ширина здания от глубины участка с отступом
    float availableDepth = std::max(0.0f, static_cast<float>(availableArea.width()) - 10.0f);   // глубина здания от длины участка с отступом

    // Ограничиваем размеры доступного пространства (минимум 10 для корректного размещения)
    availableWidth = std::max(10.0f, availableWidth);
    availableDepth = std::max(10.0f, availableDepth);

    // Создаем список возможных зданий с их размерами
    struct BuildingTemplate {
        BuildingType type;
        float minWidth, maxWidth;
        float minDepth, maxDepth;
        float minHeight, maxHeight;
    };

    std::vector<BuildingTemplate> templates = {
        {BuildingType::Low, 10.0f, 30.0f, 10.0f, 40.0f, 10.0f, 25.0f},    // низкое здание
        {BuildingType::Mid, 15.0f, 50.0f, 15.0f, 70.0f, 20.0f, 40.0f},    // среднее здание
        {BuildingType::High, 20.0f, 80.0f, 20.0f, 100.0f, 30.0f, 80.0f}   // высокое здание
    };

    // Находим все шаблоны, которые помещаются в доступное пространство
    std::vector<BuildingTemplate> fittingTemplates;
    for (const auto& tmpl : templates) {
        if (tmpl.maxWidth <= availableWidth && tmpl.maxDepth <= availableDepth) {
            fittingTemplates.push_back(tmpl);
        }
    }

    // Если ни один шаблон не помещается, используем минимальные размеры
    if (fittingTemplates.empty()) {
        // Выбираем случайное здание, уменьшая его до размеров доступного пространства
        static int buildingCounter = 0;
        buildingCounter++;
        
        int randType = (static_cast<int>(availableArea.x() + availableArea.y()) + buildingCounter) % 3;
        BuildingType type = static_cast<BuildingType>(randType);
        
        float width = std::min(availableWidth, 30.0f);
        float depth = std::min(availableDepth, 40.0f);
        float height = std::min(25.0f, std::min(width, depth) * 0.8f); // высота пропорциональна ширине
        
        switch (type) {
            case BuildingType::High:  return createHighRise(width, depth, height);
            case BuildingType::Mid:   return createMidRise(width, depth, height);
            default:                  return createLowRise(width, depth, height);
        }
    }

    // Случайно выбираем подходящий шаблон
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, static_cast<int>(fittingTemplates.size()) - 1);
    auto chosenTemplate = fittingTemplates[dist(gen)];
    
    // Генерируем случайные размеры в пределах шаблона, ограниченные доступным пространством
    std::uniform_real_distribution<float> widthDist(chosenTemplate.minWidth, 
        std::min(chosenTemplate.maxWidth, availableWidth));
    std::uniform_real_distribution<float> depthDist(chosenTemplate.minDepth, 
        std::min(chosenTemplate.maxDepth, availableDepth));
    std::uniform_real_distribution<float> heightDist(chosenTemplate.minHeight, chosenTemplate.maxHeight);

    float width = widthDist(gen);
    float depth = depthDist(gen);
    float height = heightDist(gen);

    switch (chosenTemplate.type) {
        case BuildingType::High:  return createHighRise(width, depth, height);
        case BuildingType::Mid:   return createMidRise(width, depth, height);
        default:                  return createLowRise(width, depth, height);
    }
}

GraphicObject SimpleBuildingSelector::createLowRise(float width, float depth, float height) const {
    GraphicObject obj;
    QColor color(180, 160, 100); // бежевый
    QColor roof(230, 230, 230);

    // Нижнее основание (Y=0)
    obj.AddPoint(QVector3D(0, 0, 0));
    obj.AddPoint(QVector3D(width, 0, 0));
    obj.AddPoint(QVector3D(width, 0, depth));
    obj.AddPoint(QVector3D(0, 0, depth));

    // Верхнее основание (Y=height)
    obj.AddPoint(QVector3D(0, height, 0));
    obj.AddPoint(QVector3D(width, height, 0));
    obj.AddPoint(QVector3D(width, height, depth));
    obj.AddPoint(QVector3D(0, height, depth));

    // Фасад (XZ, Y=0..height)
    obj.AddFace(0, 1, 5, color); // перед
    obj.AddFace(0, 5, 4, color);
    obj.AddFace(1, 2, 6, color); // правый
    obj.AddFace(1, 6, 5, color);
    obj.AddFace(2, 3, 7, color); // зад
    obj.AddFace(2, 7, 6, color);
    obj.AddFace(3, 0, 4, color); // левый
    obj.AddFace(3, 4, 7, color);
    obj.AddFace(4, 5, 6, roof); // крыша
    obj.AddFace(4, 6, 7, roof);

    return obj;
}

GraphicObject SimpleBuildingSelector::createMidRise(float width, float depth, float height) const {
    GraphicObject obj;
    QColor color(100, 120, 180); // голубоватый
    QColor roof(230, 230, 230);


    obj.AddPoint(QVector3D(0, 0, 0));
    obj.AddPoint(QVector3D(width, 0, 0));
    obj.AddPoint(QVector3D(width, 0, depth));
    obj.AddPoint(QVector3D(0, 0, depth));
    obj.AddPoint(QVector3D(0, height, 0));
    obj.AddPoint(QVector3D(width, height, 0));
    obj.AddPoint(QVector3D(width, height, depth));
    obj.AddPoint(QVector3D(0, height, depth));

    obj.AddFace(0, 1, 5, color);
    obj.AddFace(0, 5, 4, color);
    obj.AddFace(1, 2, 6, color);
    obj.AddFace(1, 6, 5, color);
    obj.AddFace(2, 3, 7, color);
    obj.AddFace(2, 7, 6, color);
    obj.AddFace(3, 0, 4, color);
    obj.AddFace(3, 4, 7, color);
    obj.AddFace(4, 5, 6, roof);
    obj.AddFace(4, 6, 7, roof);


    return obj;
}

GraphicObject SimpleBuildingSelector::createHighRise(float width, float depth, float height) const {
    GraphicObject obj;
    QColor color(60, 60, 70); // тёмно-серый
    QColor roof(230, 230, 230);


    obj.AddPoint(QVector3D(0, 0, 0));
    obj.AddPoint(QVector3D(width, 0, 0));
    obj.AddPoint(QVector3D(width, 0, depth));
    obj.AddPoint(QVector3D(0, 0, depth));
    obj.AddPoint(QVector3D(0, height, 0));
    obj.AddPoint(QVector3D(width, height, 0));
    obj.AddPoint(QVector3D(width, height, depth));
    obj.AddPoint(QVector3D(0, height, depth));

    obj.AddFace(0, 1, 5, color);
    obj.AddFace(0, 5, 4, color);
    obj.AddFace(1, 2, 6, color);
    obj.AddFace(1, 6, 5, color);
    obj.AddFace(2, 3, 7, color);
    obj.AddFace(2, 7, 6, color);
    obj.AddFace(3, 0, 4, color);
    obj.AddFace(3, 4, 7, color);
    obj.AddFace(4, 5, 6, roof);
    obj.AddFace(4, 6, 7, roof);


    return obj;
}

} // namespace City