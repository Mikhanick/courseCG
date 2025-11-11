#pragma once
#include <vector>
#include <QString>
#include <QSizeF>
#include <QVector3D>
#include <QColor>
#include "Face.h"

struct FloorSection {
    std::vector<QVector3D> vertices;
    std::vector<Face> faces;
};

class BuildingModel {
public:
    QString name;
    float minWidth = 10.0f, maxWidth = 30.0f;
    float minDepth = 10.0f, maxDepth = 40.0f;
    int floorCount = 3;
    float textureScale = 1.0f;
    bool fixedScale = false;
    
    FloorSection groundFloor;
    FloorSection typicalFloor;
    FloorSection roof;
};