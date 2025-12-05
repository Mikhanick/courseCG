#pragma once
#include "ScreenBuffer.h"
#include <vector>

class ZBuffer : public ScreenBuffer
{
    std::vector<float> depthData;

public:
    ZBuffer(
        int w, int h,
        CameraPtr cam,
        std::unique_ptr<ProjectionStrategy> proj) : ScreenBuffer(w, h, cam, std::move(proj))
    {
        depthData.resize(w * h);
        Clear();
    }

    void Clear() override
    {
        std::fill(depthData.begin(), depthData.end(), 1.0f);
    }

    float &At(int x, int y)
    {
        return depthData[y * width + x];
    }

    float At(int x, int y) const
    {
        return depthData[y * width + x];
    }

    inline bool ZTestAndUpdate(int x, int y, float depth)
    {
        if (!InBounds(x, y))
            return false;
        if (depth < depthData[y * width + x])
        {
            depthData[y * width + x] = depth;
            return true;
        }
        return false;
    }

    inline float GetDepthDifference(const QVector3D &worldPos) const
    {
        float x, y, depth, w;
        if (!Project(worldPos, x, y, depth, w))
        {
            return -1.0f;
        }
        int ix = static_cast<int>(x);
        int iy = static_cast<int>(y);
        if (!InBounds(ix, iy))
        {
            return -1.0f;
        }
        return depth - At(ix, iy);
    }
};
