#pragma once
#include "ScreenBuffer.h"
#include <vector>

class ShadeBuffer : public ScreenBuffer {
public:
    std::vector<float> shadeData;

    ShadeBuffer(
        int w, int h,
        CameraPtr cam,
        std::unique_ptr<ProjectionStrategy> proj
    ) : ScreenBuffer(w, h, cam, std::move(proj)) {
        shadeData.resize(w * h);
        Clear();
    }

    void Clear() override {
        std::fill(shadeData.begin(), shadeData.end(), 1.0f);
    }
};