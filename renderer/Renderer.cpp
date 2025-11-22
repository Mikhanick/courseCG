#include "Renderer.h"
#include "ZBuffer.h"
#include "ColorBuffer.h"
#include "ShadeBuffer.h"
#include "ProjectionStrategy.h"
#include <QtGui/QColor>
#include "rastrizercommand.h"
#include <vector>

Renderer::Renderer(int width, int height)
    : m_width(width), m_height(height) {
    // Буферы пока не создаём — сделаем это при первом Render
}

void Renderer::SetShadowMapSize(int width, int height) {
    m_shadowMapWidth = width;
    m_shadowMapHeight = height;
    // This will cause shadow buffers to be recreated on next render with new size
    shadowZBuf.clear(); // Clear existing shadow buffers to force recreation
}

void Renderer::UpdateShadowBuffers(const Scene& scene) {
    // Clear existing shadow buffers
    shadowZBuf.clear();

    // Calculate orthographic projection bounds based on scene extents
    float minX, minZ, maxX, maxZ;
    scene.getSceneExtent(minX, minZ, maxX, maxZ);

    // Add some margin around the scene for proper shadow coverage
    const float margin = 500.0f;  // Additional margin for shadows
    float orthoLeft = minX + margin;
    float orthoRight = maxX + margin;
    float orthoBottom = minZ + margin;
    float orthoTop = maxZ + margin;
    float orthoNear = -1000.0f;
    float orthoFar = 10000.0f;  // Keep far plane large enough to cover the scene
    qDebug() << orthoRight << orthoLeft << orthoTop << orthoBottom;
    auto ortho_proj = std::make_unique<OrthographicProjection>(
        -orthoRight, orthoLeft + 100, -orthoTop, orthoBottom, orthoNear, orthoFar
    );

    // Recreate shadow buffers with updated projection for each light
    for (auto &light: scene.lights)
    {
        shadowZBuf.push_back(light->CreateShadowZBuffer(m_shadowMapWidth, m_shadowMapHeight, ortho_proj->Clone())); // 46340 максимально

        // Render the scene from light's perspective to update shadow map
        ZBufferRasterCommand zBufCmd(shadowZBuf.rbegin()->get());
        RenderScene(scene, zBufCmd, light->GetCamera()->GetPosition(), false);
    }
}

void Renderer::EnsureBuffers(const Scene& scene) {
    if (!scene.camera) return;

    // Проверяем, нужно ли пересоздать буферы (если размеры изменились или камера/проекция)
    bool needRecreate = !m_zBuffer ||
                        m_zBuffer->GetWidth() != m_width ||
                        m_zBuffer->GetHeight() != m_height;

    if (needRecreate) {
        m_projection = std::make_unique<PerspectiveProjection>(
            m_fov, (float)m_width / m_height, m_near, m_far
            );

        // m_projection = std::make_unique<OrthographicProjection>(-16, 16, -12, 12, 0.f);
        m_zBuffer = std::make_unique<ZBuffer>(
            m_width, m_height, scene.camera, m_projection->Clone()
            );
        m_colorBuffer = std::make_unique<ColorBuffer>(
            m_width, m_height, scene.camera, m_projection->Clone()
            );
        m_shadeBuffer = std::make_unique<ShadeBuffer>(
            m_width, m_height, scene.camera, m_projection->Clone()
            );

        // Calculate orthographic projection bounds based on scene extents
        float minX, minZ, maxX, maxZ;
        scene.getSceneExtent(minX, minZ, maxX, maxZ);

        // Add some margin around the scene for proper shadow coverage
        const float margin = 50.0f;  // Additional margin for shadows
        float orthoLeft = minX - margin;
        float orthoRight = maxX + margin;
        float orthoBottom = minZ - margin;
        float orthoTop = maxZ + margin;
        float orthoNear = 0.0f;
        float orthoFar = 5000.0f;  // Keep far plane large enough to cover the scene

        auto ortho_proj = std::make_unique<OrthographicProjection>(
            orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar
        );

        // auto ortho_proj = std::make_unique<PerspectiveProjection>(120);
        for (auto &light: scene.lights)
        {
            shadowZBuf.push_back(light->CreateShadowZBuffer(m_shadowMapWidth, m_shadowMapHeight, ortho_proj->Clone())); // 46340 максимально

            ZBufferRasterCommand zBufCmd(shadowZBuf.rbegin()->get());
            RenderScene(scene, zBufCmd, light->GetCamera()->GetPosition(), false);
        }
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

void Renderer::WriteToImage(QImage& image) {
    if (image.size() != QSize(m_width, m_height)) {
        image = QImage(m_width, m_height, QImage::Format_RGB32);
    }


    // #pragma omp parallel for collapse(2)
    // for (int y = 0; y < m_height; ++y) {
    //     for (int x = 0; x < m_width; ++x) {
    //         float depth = shadowZBuf[0]->At(x, y); // Получаем значение глубины
    //         // Преобразуем глубину в оттенок серого: 0.0 -> белый (255), 1.0 -> чёрный (0)
    //         // Можно инвертировать, если хочешь: чем ближе — тем темнее
    //         int gray = qBound(0, (int)((1.0f - depth) * 200000000055.0f), 255);

    //         // Устанавливаем пиксель как оттенок серого
    //         image.setPixel(x, y, qRgb(gray, gray, gray));
    //     }
    // }
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int idx = y * m_width + x;
            float shade = m_shadeBuffer->shadeData[idx];
            QColor c = m_colorBuffer->colorData[idx];
            c = QColor(
                qBound(0, (int)(c.red() * shade * 1.3), 255),
                qBound(0, (int)(c.green() * shade * 1.1), 255),
                qBound(0, (int)(c.blue() * shade * 1.f), 255)
                );
            image.setPixel(x, y, qRgb(c.red(), c.green(), c.blue()));
        }
    }
}
