#pragma once
#include "../objects/BuildingModel.h"
#include <QString>
#include <vector>

class ModelLoader {
public:
    static std::vector<BuildingModel> loadModelsFromDirectory(const QString& dirPath);
    
private:
    static BuildingModel parseModelFile(const QString& filePath);
    static QColor parseColor(const QString& colorStr);
};