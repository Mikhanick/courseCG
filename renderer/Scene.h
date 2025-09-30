#pragma once
#include <vector>
#include <memory>
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
};