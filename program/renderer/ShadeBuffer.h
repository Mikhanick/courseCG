#pragma once
#include "ScreenBuffer.h"
#include <vector>
#include <algorithm>

class ShadeBuffer : public ScreenBuffer
{
public:
    std::vector<float> shadeData;

    ShadeBuffer(
        int w, int h,
        CameraPtr cam,
        std::unique_ptr<ProjectionStrategy> proj) : ScreenBuffer(w, h, cam, std::move(proj))
    {
        shadeData.resize(w * h);
        Clear();
    }

    void Clear() override
    {
        std::fill(shadeData.begin(), shadeData.end(), 1.0f);
    }

    void Smooth(int radius = 1)
    {
        if (radius <= 0)
            return;

        const int w = width;
        const int h = height;
        std::vector<float> newData(shadeData.size());

        if (radius == 1)
        {

#pragma omp parallel for collapse(2)
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    float sum = 4.0f * shadeData[y * w + x];
                    int count = 4;

                    if (y > 0)
                    {
                        sum += 2.0f * shadeData[(y - 1) * w + x];
                        count += 2;
                        if (x > 0)
                        {
                            sum += 1.0f * shadeData[(y - 1) * w + x - 1];
                            ++count;
                        }
                        if (x < w - 1)
                        {
                            sum += 1.0f * shadeData[(y - 1) * w + x + 1];
                            ++count;
                        }
                    }

                    if (y < h - 1)
                    {
                        sum += 2.0f * shadeData[(y + 1) * w + x];
                        count += 2;
                        if (x > 0)
                        {
                            sum += 1.0f * shadeData[(y + 1) * w + x - 1];
                            ++count;
                        }
                        if (x < w - 1)
                        {
                            sum += 1.0f * shadeData[(y + 1) * w + x + 1];
                            ++count;
                        }
                    }

                    if (x > 0)
                    {
                        sum += 2.0f * shadeData[y * w + x - 1];
                        count += 2;
                    }
                    if (x < w - 1)
                    {
                        sum += 2.0f * shadeData[y * w + x + 1];
                        count += 2;
                    }

                    newData[y * w + x] = sum / static_cast<float>(count);
                }
            }
        }
        else if (radius == 2)
        {

            static const std::vector<std::pair<int, int>> offsets = {
                {
                    0,
                    0,
                },
                {-1, 0},
                {1, 0},
                {0, -1},
                {0, 1},
                {-1, -1},
                {-1, 1},
                {1, -1},
                {1, 1},

            };

            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    float sum = 0.0f;
                    int totalWeight = 0;

                    for (int dy = -2; dy <= 2; ++dy)
                    {
                        for (int dx = -2; dx <= 2; ++dx)
                        {
                            int nx = x + dx, ny = y + dy;
                            if (nx < 0 || nx >= w || ny < 0 || ny >= h)
                                continue;

                            int dist = std::max(std::abs(dx), std::abs(dy));
                            int weight = 3 - dist;
                            sum += shadeData[ny * w + nx] * static_cast<float>(weight);
                            totalWeight += weight;
                        }
                    }
                    newData[y * w + x] = sum / static_cast<float>(totalWeight);
                }
            }
        }
        else
        {

            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    float sum = 0.0f;
                    int totalWeight = 0;
                    const int r = radius;
                    for (int dy = -r; dy <= r; ++dy)
                    {
                        for (int dx = -r; dx <= r; ++dx)
                        {
                            int nx = x + dx, ny = y + dy;
                            if (nx < 0 || nx >= w || ny < 0 || ny >= h)
                                continue;
                            int weight = r + 1 - std::max(std::abs(dx), std::abs(dy));
                            sum += shadeData[ny * w + nx] * static_cast<float>(weight);
                            totalWeight += weight;
                        }
                    }
                    newData[y * w + x] = totalWeight ? sum / static_cast<float>(totalWeight) : shadeData[y * w + x];
                }
            }
        }

        shadeData.swap(newData);
    }
};