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
        auto *curShZBuf = shadowZBuf.rbegin()->get();
        ZBufferRasterCommand zBufCmd(curShZBuf);
        RenderScene(scene, zBufCmd, light->GetCamera()->GetPosition(), false);
    }
}

void Renderer::EnsureBuffers(const Scene& scene) {
    if (!scene.camera) return;

    // Проверяем, нужно ли пересоздать буферы (если размеры изменились или камера/проекция)
    bool needRecreate = m_forceBufferRecreation ||  // Check force recreation flag
                        !m_zBuffer ||
                        m_zBuffer->GetWidth() != m_width ||
                        m_zBuffer->GetHeight() != m_height;

    if (needRecreate) {
        m_forceBufferRecreation = false;  // Reset the flag after forced recreation
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

    // Get the light multipliers from the first light source to apply to the final rendering
    // For now, using a simple approach where we store RGB multipliers directly
    // In a complete implementation, the multipliers might be passed differently
    m_lightRedMultiplier = 1.0f;
    m_lightGreenMultiplier = 1.0f;
    m_lightBlueMultiplier = 1.0f;

    if (!scene.lights.empty()) {
        if (auto* dirLight = dynamic_cast<DirectionalLight*>(scene.lights[0].get())) {
            QColor lightColor = dirLight->GetColor();
            // Use the color components as multipliers, but scale so that 255 (full RGB) = 1.0 multiplier
            // To allow values > 1.0, we can scale by a factor (e.g., allowing up to 2.0x multiplier)
            // For now use direct normalized values (0.0-1.0), but this can be adjusted for higher multipliers
            m_lightRedMultiplier = lightColor.redF();
            m_lightGreenMultiplier = lightColor.greenF();
            m_lightBlueMultiplier = lightColor.blueF();
        }
    }

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

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int idx = y * m_width + x;
            float shade = m_shadeBuffer->shadeData[idx];
            QColor c = m_colorBuffer->colorData[idx];

            // Apply light multipliers and apply the shade value
            int finalRed = qBound(0, (int)(c.red() * m_lightRedMultiplier * shade * 1.3), 255);
            int finalGreen = qBound(0, (int)(c.green() * m_lightGreenMultiplier * shade * 1.1), 255);
            int finalBlue = qBound(0, (int)(c.blue() * m_lightBlueMultiplier * shade * 1.0), 255);

            image.setPixel(x, y, qRgb(finalRed, finalGreen, finalBlue));
        }
    }
}
