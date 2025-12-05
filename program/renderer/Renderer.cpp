#include "Renderer.h"
#include "ZBuffer.h"
#include "ColorBuffer.h"
#include "ShadeBuffer.h"
#include "ProjectionStrategy.h"
#include <QtGui/QColor>
#include "rastrizercommand.h"
#include <vector>
#include "DirectionalLight.h"

Renderer::Renderer(int width, int height)
    : m_width(width), m_height(height)
{
}

void Renderer::SetShadowMapSize(int width, int height)
{
    m_shadowMapWidth = width;
    m_shadowMapHeight = height;
    shadowZBuf.clear();
}

void Renderer::UpdateShadowBuffers(const Scene &scene)
{
    shadowZBuf.clear();

    float minX, minZ, maxX, maxZ;
    scene.getSceneExtent(minX, minZ, maxX, maxZ);

    const float margin = 500.0f;
    float orthoLeft = minX + margin;
    float orthoRight = maxX + margin;
    float orthoBottom = minZ + margin;
    float orthoTop = maxZ + margin;
    float orthoNear = -1000.0f;
    float orthoFar = 10000.0f;
    qDebug() << orthoRight << orthoLeft << orthoTop << orthoBottom;
    auto ortho_proj = std::make_unique<OrthographicProjection>(
        -orthoRight, orthoLeft + 100, -orthoTop, orthoBottom, orthoNear, orthoFar);

    for (auto &light : scene.lights)
    {
        shadowZBuf.push_back(light->CreateShadowZBuffer(m_shadowMapWidth, m_shadowMapHeight, ortho_proj->Clone())); // 46340 максимально

        auto *curShZBuf = shadowZBuf.rbegin()->get();
        ZBufferRasterCommand zBufCmd(curShZBuf);
        RenderScene(scene, zBufCmd, light->GetCamera()->GetPosition(), false);
    }
}

void Renderer::EnsureBuffers(const Scene &scene)
{
    if (!scene.camera)
        return;

    bool needRecreate = m_forceBufferRecreation ||
                        !m_zBuffer ||
                        m_zBuffer->GetWidth() != m_width ||
                        m_zBuffer->GetHeight() != m_height;

    if (needRecreate)
    {
        m_forceBufferRecreation = false;
        m_projection = std::make_unique<PerspectiveProjection>(
            m_fov, (float)m_width / m_height, m_near, m_far);

        // m_projection = std::make_unique<OrthographicProjection>(-16, 16, -12, 12, 0.f);
        m_zBuffer = std::make_unique<ZBuffer>(
            m_width, m_height, scene.camera, m_projection->Clone());
        m_colorBuffer = std::make_unique<ColorBuffer>(
            m_width, m_height, scene.camera, m_projection->Clone());
        m_shadeBuffer = std::make_unique<ShadeBuffer>(
            m_width, m_height, scene.camera, m_projection->Clone());
    }

    // Очищаем буферы перед рендерингом
    m_zBuffer->Clear();
    m_colorBuffer->Clear();
    m_shadeBuffer->Clear();
}

void Renderer::Render(const Scene &scene, QImage &image)
{
    if (!scene.camera)
        return;

    EnsureBuffers(scene);

    ZBufferRasterCommand zBufCmd(m_zBuffer.get());
    ColorWriteCommand colorBufCmd(m_colorBuffer.get());
    ShadeWriteCommand shadeBuf(m_shadeBuffer.get(), &scene, shadowZBuf);
    ComplexWriteCommand complexCmd(zBufCmd, colorBufCmd, shadeBuf);

    QVector3D cameraPos = scene.camera->GetPosition();
    RenderScene(scene, complexCmd, cameraPos);
    m_shadeBuffer->Smooth(2);
    WriteToImage(image);
}

void Renderer::WriteToImage(QImage &image)
{
    if (image.width() != m_width || image.height() != m_height || image.format() != QImage::Format_RGB32)
    {
        image = QImage(m_width, m_height, QImage::Format_RGB32);
    }

    QRgb *bits = reinterpret_cast<QRgb *>(image.bits());

#pragma omp parallel for collapse(2)
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int idx = y * m_width + x;
            float shade = m_shadeBuffer->shadeData[idx];
            QColor c = m_colorBuffer->colorData[idx];

            int finalRed = qBound(0, (int)(c.red() * m_lightRedMultiplier * shade), 255);
            int finalGreen = qBound(0, (int)(c.green() * m_lightGreenMultiplier * shade), 255);
            int finalBlue = qBound(0, (int)(c.blue() * m_lightBlueMultiplier * shade), 255);

            bits[idx] = qRgb(finalRed, finalGreen, finalBlue);
        }
    }
}
