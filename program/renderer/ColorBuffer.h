#pragma once
#include "ScreenBuffer.h"
#include <QtGui/QColor>
#include <vector>

class ColorBuffer : public ScreenBuffer {
public:
    std::vector<QColor> colorData;

    ColorBuffer(
        int w, int h,
        CameraPtr cam,
        std::unique_ptr<ProjectionStrategy> proj
    ) : ScreenBuffer(w, h, cam, std::move(proj)) {
        colorData.resize(w * h);
        _clear();
    }

    void Clear() override {
        _clear();
    }
private:
    void _clear(){
        std::fill(colorData.begin(), colorData.end(), QColor(130, 200, 229));
    }
};
