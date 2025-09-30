#pragma once
#include <QtGui/QVector3D>
#include <memory>
#include "Camera.h"
#include "ProjectionStrategy.h"

class ScreenBuffer {
protected:
    int width, height;
    CameraPtr camera;
    std::unique_ptr<ProjectionStrategy> projection;

public:
    ScreenBuffer(
        int w, int h,
        CameraPtr cam,
        std::unique_ptr<ProjectionStrategy> proj
        ) : width(w), height(h), camera(cam), projection(std::move(proj)) {}

    virtual ~ScreenBuffer() = default;

    virtual void Clear() = 0;

    // Изменённая версия: возвращает outX, outY, outDepth, И outW (clip.w)
    bool Project(
        const QVector3D& worldPos,
        float& outX,
        float& outY,
        float& outDepth,
        float& outW
        ) const {
        if (!camera || !projection) return false;

        float clipX, clipY, clipZ, clipW;
        if (!projection->Project(worldPos, *camera, clipX, clipY, clipZ, clipW)) {
            return false;
        }

        // Проверка на деление на ноль
        if (clipW == 0.0f) return false;

        // Преобразуем в NDC
        float ndcX = clipX / clipW;
        float ndcY = clipY / clipW;
        float ndcZ = clipZ / clipW;

        // Отсечение в NDC — опционально, но рекомендуется
        if (ndcZ < -1.0f || ndcZ > 1.0f) {
            return false;
        }

        // Преобразуем NDC → экран
        outX = (ndcX + 1.0f) * 0.5f * width;     // [-1,1] → [0,width]
        outY = (1.0f - (ndcY + 1.0f) * 0.5f) * height; // [-1,1] → [0,height], инверсия Y
        outDepth = (ndcZ + 1.0f) * 0.5f;         // [-1,1] → [0,1]

        // Возвращаем исходный w — нужен для интерполяции!
        outW = clipW;

        return true;
    }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool InBounds(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};
