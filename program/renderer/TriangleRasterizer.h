#pragma once
#include <QtGui/QVector3D>
#include <QtGui/QColor>
#include "rastrizercommand.h"
#include <cmath>

class TriangleRasterizer {
public:
    TriangleRasterizer() = default;

    template<RasterCommand_v RasterCommand>
    void DrawTriangle(const QVector3D &p0,
                      const QVector3D &p1,
                      const QVector3D &p2,
                      RasterCommand &cmd);

private:
    bool IsTriangleOutsideViewport(
        const QVector3D& p0,
        const QVector3D& p1,
        const QVector3D& p2,
        const QRectF& viewport
        );
};


// template<RasterCommand_v RasterCommand>
// void TriangleRasterizer::DrawTriangle(const QVector3D &p0,
//                                       const QVector3D &p1,
//                                       const QVector3D &p2,
//                                       RasterCommand &cmd)
// {
//     float x0, y0, z0, x1, y1, z1, x2, y2, z2;
//     if (!cmd.Project(p0, x0, y0, z0) || !cmd.Project(p1, x1, y1, z1)
//         || !cmd.Project(p2, x2, y2, z2)) {
//         return;
//     }
//     QRectF viewport = cmd.GetViewport();
//     if (IsTriangleOutsideViewport(QVector3D(x0, y0, z0),
//                                   QVector3D(x1, y1, z1),
//                                   QVector3D(x2, y2, z2),
//                                   viewport)) {
//         return;
//     }

//     float minXf = std::min({x0, x1, x2});
//     float maxXf = std::max({x0, x1, x2});
//     float minYf = std::min({y0, y1, y2});
//     float maxYf = std::max({y0, y1, y2});

//     int minX = std::max((int)viewport.left(), (int) std::floor(minXf));
//     int maxX = std::min((int)viewport.right(), (int) std::ceil(maxXf));
//     int minY = std::max((int)viewport.top(), (int) std::floor(minYf));
//     int maxY = std::min((int)viewport.bottom(), (int) std::ceil(maxYf));

//     auto edge = [](float ax, float ay, float bx, float by, float cx, float cy) {
//         return (cx - ax) * (by - ay) - (cy - ay) * (bx - ax);
//     };

//     for (int y = minY; y <= maxY; ++y) {
//         for (int x = minX; x <= maxX; ++x) {
//             float w0 = edge(x1, y1, x2, y2, x + 0.5f, y + 0.5f);
//             float w1 = edge(x2, y2, x0, y0, x + 0.5f, y + 0.5f);
//             float w2 = edge(x0, y0, x1, y1, x + 0.5f, y + 0.5f);

//             if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
//                 float sum = w0 + w1 + w2;
//                 if (sum == 0)
//                     continue;
//                 float invSum = 1.0f / sum;
//                 w0 *= invSum;
//                 w1 *= invSum;
//                 w2 *= invSum;
//                 FragmentData fragment{x,
//                                       y,
//                                       z0 * w0 + z1 * w1 + z2 * w2,
//                                       p0 * w0 + p1 * w1 + p2 * w2};

//                 cmd.Execute(fragment);
//             }
//         }
//     }
// }

template<RasterCommand_v RasterCommand>
void TriangleRasterizer::DrawTriangle(const QVector3D &p0,
                                      const QVector3D &p1,
                                      const QVector3D &p2,
                                      RasterCommand &cmd)
{
    float x0, y0, z0, w0, x1, y1, z1, w1, x2, y2, z2, w2;

    // Проектируем вершины и получаем w (до деления на w!)
    if (!cmd.Project(p0, x0, y0, z0, w0) ||
        !cmd.Project(p1, x1, y1, z1, w1) ||
        !cmd.Project(p2, x2, y2, z2, w2)) {
        return;
    }

    QRectF viewport = cmd.GetViewport();
    if (IsTriangleOutsideViewport(QVector3D(x0, y0, z0),
                                  QVector3D(x1, y1, z1),
                                  QVector3D(x2, y2, z2),
                                  viewport)) {
        return;
    }

    float minXf = std::min({x0, x1, x2});
    float maxXf = std::max({x0, x1, x2});
    float minYf = std::min({y0, y1, y2});
    float maxYf = std::max({y0, y1, y2});

    int minX = std::max((int)viewport.left(),  (int)std::floor(minXf));
    int maxX = std::min((int)viewport.right(), (int)std::ceil(maxXf));
    int minY = std::max((int)viewport.top(),   (int)std::floor(minYf));
    int maxY = std::min((int)viewport.bottom(),(int)std::ceil(maxYf));

    auto edge = [](float ax, float ay, float bx, float by, float cx, float cy) {
        return (cx - ax) * (by - ay) - (cy - ay) * (bx - ax);
    };

    // Предварительно вычисляем 1/w и атрибуты, делённые на w
    float invW0 = 1.0f / w0;
    float invW1 = 1.0f / w1;
    float invW2 = 1.0f / w2;

    QVector3D p0_over_w = p0 * invW0;
    QVector3D p1_over_w = p1 * invW1;
    QVector3D p2_over_w = p2 * invW2;

    #pragma omp parallel for collapse(2)
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float sampleX = x + 0.5f;
            float sampleY = y + 0.5f;

            float w0 = edge(x1, y1, x2, y2, sampleX, sampleY);
            float w1 = edge(x2, y2, x0, y0, sampleX, sampleY);
            float w2 = edge(x0, y0, x1, y1, sampleX, sampleY);

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float sum = w0 + w1 + w2;
                if (sum == 0) continue;
                float invSum = 1.0f / sum;
                w0 *= invSum;
                w1 *= invSum;
                w2 *= invSum;

                // Линейно интерполируем 1/w и p/w
                float interpolated_invW = invW0 * w0 + invW1 * w1 + invW2 * w2;
                QVector3D interpolated_p_over_w = p0_over_w * w0 + p1_over_w * w1 + p2_over_w * w2;

                // Восстанавливаем правильную мировую позицию
                float finalW = 1.0f / interpolated_invW;
                QVector3D worldPos = interpolated_p_over_w * finalW;

                // Z-буфер: можно интерполировать либо линейно z (в NDC), либо перспективно — зависит от системы
                // Здесь для простоты оставим линейную интерполяцию z (в NDC-пространстве, после /w)
                float depth = z0 * w0 + z1 * w1 + z2 * w2;

                FragmentData fragment{x, y, depth, worldPos};
                cmd.Execute(fragment);
            }
        }
    }
}
