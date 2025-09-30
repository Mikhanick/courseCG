#pragma once
#include <QtGui/QImage>
#include "Scene.h"
#include "ZBuffer.h"
#include "ColorBuffer.h"
#include "ShadeBuffer.h"
#include "ProjectionStrategy.h"
#include <memory>
#include "rastrizercommand.h"
#include "TriangleRasterizer.h"

class Renderer
{
    int m_width;
    int m_height;
    float m_fov = 90.0f;
    float m_near = 0.1f;
    float m_far = 100.0f;

    std::unique_ptr<ZBuffer> m_zBuffer;
    std::unique_ptr<ColorBuffer> m_colorBuffer;
    std::unique_ptr<ShadeBuffer> m_shadeBuffer;
    std::unique_ptr<ProjectionStrategy> m_projection;

    std::vector<std::unique_ptr<ZBuffer>> shadowZBuf;

public:
    Renderer(int width, int height);
    void Render(const Scene &scene, QImage &image);

private:
    void EnsureBuffers(const Scene &scene);
    void WriteToImage(QImage &image);

    template<RasterCommand_v Command>
    void RenderScene(const Scene &scene, Command &command, const QVector3D &cameraPos, bool needsBackCool = true);
};


template<RasterCommand_v Command>
void Renderer::RenderScene(const Scene &scene, Command &command,const QVector3D &cameraPos, bool needBackCool)
{
    if (!scene.camera)
        return;

    TriangleRasterizer rasterizer;

    for (const auto &obj : scene.objects) {
        for (const auto &face : obj->faces) {
            if (obj->points.empty())
                continue;

            // Backface culling
            if (needBackCool)
            {
                QVector3D viewDir = (obj->points[face.index0] - cameraPos).normalized();
                if (QVector3D::dotProduct(face.normal, viewDir) > 0)
                    continue;
            }

            // Проверка индексов
            if (static_cast<size_t>(face.index0) >= obj->points.size()
                || static_cast<size_t>(face.index1) >= obj->points.size()
                || static_cast<size_t>(face.index2) >= obj->points.size()) {
                continue;
            }

            QVector3D p0 = obj->points[face.index0];
            QVector3D p1 = obj->points[face.index1];
            QVector3D p2 = obj->points[face.index2];

            command.setColor(face.color);
            command.setNormal(face.normal);

            rasterizer.DrawTriangle(p0, p1, p2, command);
        }
    }
}
