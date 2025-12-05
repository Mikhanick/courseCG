#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include "GraphicObject.h"
#include "Light.h"
#include "Camera.h"

class Scene {
public:
    std::vector<std::unique_ptr<GraphicObject>> objects;
    CameraPtr camera;
    std::vector<std::unique_ptr<Light>> lights;

    void AddObject(GraphicObject* obj) {
        if (obj) objects.emplace_back(obj);
    }

    void AddLight(Light* light) {
        if (light) lights.emplace_back(light);
    }

    void MarkAllShadowMapsDirty() {
        for (auto& light : lights) {
            light->MarkShadowMapDirty();
        }
    }

    void getBounds(float& minX, float& minY, float& minZ, float& maxX, float& maxY, float& maxZ) const {
        if (objects.empty()) {
            minX = minY = minZ = -10.0f;
            maxX = maxY = maxZ = 10.0f;
            return;
        }

        minX = minY = minZ = std::numeric_limits<float>::max();
        maxX = maxY = maxZ = std::numeric_limits<float>::lowest();

        for (const auto& obj : objects) {
            if (obj) {
                for (const auto& point : obj->points) {
                    minX = std::min(minX, point.x());
                    minY = std::min(minY, point.y());
                    minZ = std::min(minZ, point.z());
                    maxX = std::max(maxX, point.x());
                    maxY = std::max(maxY, point.y());
                    maxZ = std::max(maxZ, point.z());
                }
            }
        }
    }

    void getSceneExtent(float& minX, float& minZ, float& maxX, float& maxZ) const {
        if (objects.empty()) {
            minX = minZ = -50.0f;
            maxX = maxZ = 50.0f;
            return;
        }

        minX = minZ = std::numeric_limits<float>::max();
        maxX = maxZ = std::numeric_limits<float>::lowest();

        for (const auto& obj : objects) {
            if (obj) {
                for (const auto& point : obj->points) {
                    minX = std::min(minX, point.x());
                    minZ = std::min(minZ, point.z());
                    maxX = std::max(maxX, point.x());
                    maxZ = std::max(maxZ, point.z());
                }
            }
        }
    }
};