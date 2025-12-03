#pragma once
#include "ScreenBuffer.h"
#include <vector>
#include <algorithm>

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

    void Smooth(int radius = 1) {
    if (radius <= 0) return;

    const int w = width;
    const int h = height;
    std::vector<float> newData(shadeData.size());

    // Выбираем реализацию по radius — без ветвлений внутри пикселей
    if (radius == 1) {
        // Ядро 3×3: веса по чебышёву: центр=2, крест=1, углы=1 → но оптимизируем явно
        // Фактически: вес центра = 2, все соседи = 1 → сумма весов = 2 + 8*1 = 10
        // Но для симметрии и простоты — используем веса: центр=4, кардиналы=2, углы=1 → сумма=16 (как box-filter ×2)
        // Однако для максимальной скорости — делаем вручную, без вложенных циклов:
        #pragma omp parallel for collapse(2)
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                float sum = 4.0f * shadeData[y * w + x];  // центр ×4
                int count = 4;

                // верх
                if (y > 0) {
                    sum += 2.0f * shadeData[(y-1) * w + x]; count += 2;
                    if (x > 0)     { sum += 1.0f * shadeData[(y-1) * w + x-1]; ++count; }
                    if (x < w-1)   { sum += 1.0f * shadeData[(y-1) * w + x+1]; ++count; }
                }
                // низ
                if (y < h-1) {
                    sum += 2.0f * shadeData[(y+1) * w + x]; count += 2;
                    if (x > 0)     { sum += 1.0f * shadeData[(y+1) * w + x-1]; ++count; }
                    if (x < w-1)   { sum += 1.0f * shadeData[(y+1) * w + x+1]; ++count; }
                }
                // лево/право (без углов — уже учтены)
                if (x > 0)     { sum += 2.0f * shadeData[y * w + x-1]; count += 2; }
                if (x < w-1)   { sum += 2.0f * shadeData[y * w + x+1]; count += 2; }

                newData[y * w + x] = sum / static_cast<float>(count);
            }
        }
    } else if (radius == 2) {
        // Ядро 5×5, веса = 3 − max(|dx|,|dy|): центр=3, кольца: 2, 1
        // Веса:  
        // 1 1 1 1 1
        // 1 2 2 2 1
        // 1 2 3 2 1
        // 1 2 2 2 1
        // 1 1 1 1 1
        // Сумма = 3 + 4*2 + 8*2 + 12*1 = 3 + 8 + 16 + 12 = 39? → пересчитаем:
        // Лучше — предвычислим смещения и веса один раз:
        static const std::vector<std::pair<int, int>> offsets = {
            { 0, 0,}, // w=3
            {-1, 0},{1, 0},{0,-1},{0,1}, // w=2 (×4)
            {-1,-1},{-1,1},{1,-1},{1,1}, // w=2 (×4) — нет, это тоже 2!
            // На самом деле при radius=2: weight = 3 - max(|dx|,|dy|)
            // max=0 → w=3 (1 шт)
            // max=1 → w=2 (8 шт: квадрат 3×3 без центра)
            // max=2 → w=1 (16 шт: квадрат 5×5 без 3×3)
        };
        // Чтобы не усложнять — используем двойной цикл, но с целочисленными весами и без float в цикле:
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                float sum = 0.0f;
                int totalWeight = 0;

                for (int dy = -2; dy <= 2; ++dy) {
                    for (int dx = -2; dx <= 2; ++dx) {
                        int nx = x + dx, ny = y + dy;
                        if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

                        int dist = std::max(std::abs(dx), std::abs(dy));
                        int weight = 3 - dist; // 3, 2, or 1
                        sum += shadeData[ny * w + nx] * static_cast<float>(weight);
                        totalWeight += weight;
                    }
                }
                newData[y * w + x] = sum / static_cast<float>(totalWeight);
            }
        }
    } else {
        // fallback: общий случай (редко используется, но на всякий)
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                float sum = 0.0f;
                int totalWeight = 0;
                const int r = radius;
                for (int dy = -r; dy <= r; ++dy) {
                    for (int dx = -r; dx <= r; ++dx) {
                        int nx = x + dx, ny = y + dy;
                        if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
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