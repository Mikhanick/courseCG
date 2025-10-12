#ifndef RASTERIZERCOMMAND_H
#define RASTERIZERCOMMAND_H

#include <QRectF>
#include "ZBuffer.h"
#include "ColorBuffer.h"
#include <concepts>
#include "ShadeBuffer.h"
#include "Scene.h"
#include <cmath>

struct FragmentData
{
    int x;
    int y;
    float depth;
    QVector3D worldPos;
};

// ➤ Обновлённый концепт: Project теперь требует 4 параметра (включая w)
template<typename T>
concept RasterCommand_v = requires(T cmd,
                                   const QVector3D &worldPos,
                                   float &x,
                                   float &y,
                                   float &z,
                                   float &w,           // ← ДОБАВЛЕНО
                                   const FragmentData &frag,
                                   QColor color,
                                   QVector3D vec) {
    // 1. Должен уметь возвращать viewport
    { cmd.GetViewport() } -> std::same_as<QRectF>;

    // 2. Должен уметь проецировать вершину — с 4 параметрами!
    { cmd.Project(worldPos, x, y, z, w) } -> std::same_as<bool>;

    // 3. Должен уметь выполнять фрагмент
    { cmd.Execute(frag) } -> std::same_as<bool>;

    { cmd.setColor(color) } -> std::same_as<void>;

    { cmd.setNormal(vec) } -> std::same_as<void>;
};

// ➤ Z-буферная команда
class ZBufferRasterCommand
{
    ZBuffer *zBuffer;

public:
    ZBufferRasterCommand(ZBuffer *zb)
        : zBuffer(zb)
    {
        if (!zBuffer)
            throw std::invalid_argument("ZBuffer cannot be null");
    }

    QRectF GetViewport() const { return QRectF(0, 0, zBuffer->GetWidth(), zBuffer->GetHeight()); }

    // ✅ Адаптировано: добавлен параметр w
    bool Project(const QVector3D &worldPos, float &x, float &y, float &z, float &w)
    {
        return zBuffer->Project(worldPos, x, y, z, w);
    }

    inline bool Execute(const FragmentData &frag)
    {
        return zBuffer && zBuffer->ZTestAndUpdate(frag.x, frag.y, frag.depth);
    }

    inline void setColor(const QColor &clr) {}
    inline void setNormal(const QVector3D &vec) {}
};

// ➤ Команда записи цвета
class ColorWriteCommand
{
    ColorBuffer *colorBuffer;
    QColor color;

public:
    ColorWriteCommand(ColorBuffer *cb, const QColor &c = QColor(200, 200, 200))
        : colorBuffer(cb)
        , color(c)
    {
        if (!colorBuffer)
            throw std::invalid_argument("ColorBuffer cannot be null");
    }

    QRectF GetViewport() const
    {
        return QRectF(0, 0, colorBuffer->GetWidth(), colorBuffer->GetHeight());
    }

    // ✅ Адаптировано: добавлен параметр w
    bool Project(const QVector3D &worldPos, float &x, float &y, float &z, float &w)
    {
        return colorBuffer->Project(worldPos, x, y, z, w);
    }

    bool Execute(const FragmentData &frag)
    {
        if (colorBuffer->InBounds(frag.x, frag.y)) {
            colorBuffer->colorData[frag.y * colorBuffer->GetWidth() + frag.x] = color;
        }
        return true;
    }

    void setColor(const QColor &clr) { color = clr; }
    void setNormal(const QVector3D &vec) {}
};

// ➤ Команда шейдинга (освещения)
class ShadeWriteCommand
{
    ShadeBuffer *shadeBuffer;
    const Scene *scene;
    const std::vector<std::unique_ptr<ZBuffer>> &shadowZBuffers;
    QVector3D normal;

public:
    ShadeWriteCommand(ShadeBuffer *sb,
                      const Scene *s,
                      const std::vector<std::unique_ptr<ZBuffer>> &shadows = {},
                      const QVector3D &n = QVector3D(0, 0, 1))
        : shadeBuffer(sb)
        , scene(s)
        , shadowZBuffers(shadows)
        , normal(n.normalized())
    {
        if (!shadeBuffer)
            throw std::invalid_argument("ShadeBuffer cannot be null");
        if (!scene)
            throw std::invalid_argument("Scene cannot be null");
    }

    QRectF GetViewport() const
    {
        return QRectF(0, 0, shadeBuffer->GetWidth(), shadeBuffer->GetHeight());
    }

    // ✅ Адаптировано: добавлен параметр w
    bool Project(const QVector3D &worldPos, float &x, float &y, float &z, float &w)
    {
        return shadeBuffer->Project(worldPos, x, y, z, w);
    }

    bool Execute(const FragmentData &frag)
    {
        if (!scene || !shadeBuffer)
            return false;

        float maxShade = 0.0f;

        for (std::size_t i = 0; i < scene->lights.size(); ++i) {
            Light *light = scene->lights[i].get();
            ZBuffer *shadowZBuffer = (i < shadowZBuffers.size()) ? shadowZBuffers[i].get() : nullptr;

            QVector3D lightDir = light->GetDirectionTo(frag.worldPos).normalized();
            float NdotL = std::max(0.6f, QVector3D::dotProduct(normal, -lightDir));

            float shadowFactor = 1.0f;
            if (QVector3D::dotProduct(normal, lightDir) > 0) {
                shadowFactor = 0.35f;
            } else if (shadowZBuffer) {
                shadowFactor = light->ComputeShadowFactor(frag.worldPos,
                                                          shadowZBuffer,
                                                          light->GetCamera().get());
            }

            float shade = fmin(NdotL, shadowFactor);
            maxShade = std::max(maxShade, shade);
        }

        if (shadeBuffer->InBounds(frag.x, frag.y)) {
            shadeBuffer->shadeData[frag.y * shadeBuffer->GetWidth() + frag.x] = maxShade;
        }

        return true;
    }

    void setNormal(const QVector3D &n) { normal = n.normalized(); }
    void setColor(const QColor &) {} // игнорируем — не нужно для шейдинга
};

// ➤ Комплексная команда (Z + Color + Shade)
class ComplexWriteCommand
{
    ZBufferRasterCommand &zCommand;
    ColorWriteCommand &colorWriter;
    ShadeWriteCommand &shadeWriter;

public:
    ComplexWriteCommand(ZBufferRasterCommand &zcmd, ColorWriteCommand &cw, ShadeWriteCommand &sw)
        : zCommand(zcmd)
        , colorWriter(cw)
        , shadeWriter(sw)
    {}

    QRectF GetViewport() const { return zCommand.GetViewport(); }

    // ✅ Адаптировано: добавлен параметр w, делегируем zCommand
    bool Project(const QVector3D &worldPos, float &x, float &y, float &z, float &w)
    {
        return zCommand.Project(worldPos, x, y, z, w);
    }

    bool Execute(const FragmentData &frag)
    {
        if (!zCommand.Execute(frag)) {
            return false;
        }

        return colorWriter.Execute(frag)
               && shadeWriter.Execute(frag);
    }

    void setColor(const QColor &clr) { colorWriter.setColor(clr); }
    void setNormal(const QVector3D &vec) { shadeWriter.setNormal(vec); }
};

#endif // RASTERIZERCOMMAND_H
