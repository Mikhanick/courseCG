#include "ModelLoader.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <stdexcept>

std::vector<BuildingModel> ModelLoader::loadModelsFromDirectory(const QString& dirPath) {
    std::vector<BuildingModel> models;
    QDir dir(dirPath);
    
    if (!dir.exists()) {
        qWarning() << "Models directory does not exist:" << dirPath;
        return models;
    }
    
    const QStringList modelFiles = dir.entryList({"*.json"}, QDir::Files);
    if (modelFiles.isEmpty()) {
        qWarning() << "No JSON model files found in directory:" << dirPath;
        return models;
    }
    
    for (const QString& fileName : modelFiles) {
        QString filePath = dir.filePath(fileName);
        try {
            BuildingModel model = parseModelFile(filePath);
            models.push_back(model);
            qDebug() << "Loaded building model:" << model.name;
        } catch (const std::exception& e) {
            qWarning() << "Error loading model" << fileName << ":" << e.what();
        }
    }
    
    return models;
}

BuildingModel ModelLoader::parseModelFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open file: " + filePath.toStdString());
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        throw std::runtime_error("JSON parse error: " + parseError.errorString().toStdString());
    }
    
    if (!doc.isObject()) {
        throw std::runtime_error("Invalid JSON format: expected object");
    }
    
    BuildingModel model;
    model.name = QFileInfo(filePath).baseName();
    QJsonObject root = doc.object();
    
    // Загружаем размеры
    if (root.contains("dimensions") && root["dimensions"].isObject()) {
        QJsonObject dims = root["dimensions"].toObject();
        model.fixedScale = dims["fixed_scale"].toBool(false); 
        model.minWidth = dims["min_width"].toDouble(10.0);
        model.maxWidth = dims["max_width"].toDouble(30.0);
        model.minDepth = dims["min_depth"].toDouble(10.0);
        model.maxDepth = dims["max_depth"].toDouble(40.0);
    }
    
    // Загружаем параметры этажей
    if (root.contains("floors") && root["floors"].isObject()) {
        QJsonObject floors = root["floors"].toObject();
        model.floorCount = qBound(1, floors["count"].toInt(3), 60); // Ограничиваем разумными пределами
        model.textureScale = floors["texture_scale"].toDouble(1.0);
    }
    
    // Вспомогательная функция для загрузки секции
    auto loadSection = [&](const QString& sectionName) -> FloorSection {
        FloorSection section;
        
        if (!root.contains(sectionName) || !root[sectionName].isObject()) {
            throw std::runtime_error("Missing required section: " + sectionName.toStdString());
        }
        
        QJsonObject sectionObj = root[sectionName].toObject();
        
        // Загружаем вершины
        if (!sectionObj.contains("vertices") || !sectionObj["vertices"].isArray()) {
            throw std::runtime_error("Section " + sectionName.toStdString() + " missing vertices array");
        }
        
        QJsonArray verticesArr = sectionObj["vertices"].toArray();
        for (int i = 0; i < verticesArr.size(); ++i) {
            QJsonValue val = verticesArr[i];
            if (!val.isArray() || val.toArray().size() < 3) {
                throw std::runtime_error("Invalid vertex format at index " + std::to_string(i));
            }
            
            QJsonArray v = val.toArray();
            section.vertices.push_back(QVector3D(
                v[0].toDouble(),
                v[1].toDouble(),
                v[2].toDouble()
            ));
        }
        
        // Загружаем грани
        if (!sectionObj.contains("faces") || !sectionObj["faces"].isArray()) {
            throw std::runtime_error("Section " + sectionName.toStdString() + " missing faces array");
        }
        
        QJsonArray facesArr = sectionObj["faces"].toArray();
        for (int i = 0; i < facesArr.size(); ++i) {
            QJsonValue val = facesArr[i];
            if (!val.isArray() || val.toArray().size() < 4) {
                throw std::runtime_error("Invalid face format at index " + std::to_string(i));
            }
            
            QJsonArray f = val.toArray();
            if (f[0].toInt() >= section.vertices.size() ||
                f[1].toInt() >= section.vertices.size() ||
                f[2].toInt() >= section.vertices.size()) {
                throw std::runtime_error("Face references invalid vertex index");
            }
            
            section.faces.emplace_back(
                f[0].toInt(),
                f[1].toInt(),
                f[2].toInt(),
                parseColor(f[3].toString())
            );
        }
        
        return section;
    };
    
    // Загружаем обязательные секции
    model.groundFloor = loadSection("ground_floor");
    model.typicalFloor = loadSection("typical_floor");
    model.roof = loadSection("roof");
    
    return model;
}

QColor ModelLoader::parseColor(const QString& colorStr) {
    if (colorStr.startsWith('#')) {
        QColor color(colorStr);
        if (color.isValid()) return color;
    }
    
    // Стандартные цвета как fallback
    static const std::map<QString, QColor> namedColors = {
        {"beige", QColor(180, 160, 100)},
        {"skyblue", QColor(100, 120, 180)},
        {"darkgray", QColor(60, 60, 70)},
        {"gray", Qt::gray},
        {"white", Qt::white}
    };
    
    auto it = namedColors.find(colorStr.toLower());
    if (it != namedColors.end()) return it->second;
    
    qWarning() << "Invalid color format, using default:" << colorStr;
    return QColor(200, 200, 200); // Светло-серый по умолчанию
}