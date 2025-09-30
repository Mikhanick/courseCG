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

        auto ortho_proj = std::make_unique<OrthographicProjection>(-50, 50, -50, 50, 0.f, 100.f);
        // auto ortho_proj = std::make_unique<PerspectiveProjection>();
        for (auto &light: scene.lights)
        {
            shadowZBuf.push_back(light->CreateShadowZBuffer(12000, 12000, ortho_proj->Clone()));

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

    // ✅ Создаём команду для основного рендера
    ZBufferRasterCommand zBufCmd(m_zBuffer.get());
    ColorWriteCommand colorBufCmd(m_colorBuffer.get());
    ShadeWriteCommand shadeBuf(m_shadeBuffer.get(), &scene, shadowZBuf);
    ComplexWriteCommand complexCmd(zBufCmd, colorBufCmd, shadeBuf);

    // ✅ Передаём команду в шаблонный RenderScene
    QVector3D cameraPos = scene.camera->GetPosition();
    RenderScene(scene, complexCmd, cameraPos);

    WriteToImage(image);
}

void Renderer::WriteToImage(QImage& image) {
    if (image.size() != QSize(m_width, m_height)) {
        image = QImage(m_width, m_height, QImage::Format_RGB32);
    }

    // for (int y = 0; y < m_height; ++y) {
    //     for (int x = 0; x < m_width; ++x) {
    //         float depth = shadowZBuf[0]->At(x, y); // Получаем значение глубины

    //         // Преобразуем глубину в оттенок серого: 0.0 -> белый (255), 1.0 -> чёрный (0)
    //         // Можно инвертировать, если хочешь: чем ближе — тем темнее
    //         int gray = qBound(0, (int)((1.0f - depth) * 255.0f), 255);

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
                qBound(0, (int)(c.green() * shade), 255),
                qBound(0, (int)(c.blue() * shade), 255)
                );
            image.setPixel(x, y, qRgb(c.red(), c.green(), c.blue()));
        }
    }
}
