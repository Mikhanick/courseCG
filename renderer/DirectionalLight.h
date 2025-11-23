#pragma once
#include "Light.h"
#include <QtGui/QVector3D>
#include <QtGui/QColor>

class DirectionalLight : public Light {
    QVector3D direction;
    QColor color;
    mutable bool shadowMapDirty = true;
    mutable CameraPtr cachedCamera; // кэшируем камеру света

public:
    DirectionalLight(const QVector3D& dir, const QColor& lightColor = QColor(255, 255, 255));

    void SetDirection(const QVector3D& dir);
    void SetColor(const QColor& lightColor) { color = lightColor; }
    QVector3D GetDirectionTo(const QVector3D& point) const override;
    float ComputeShadowFactor(
        const QVector3D& worldPos,
        const ZBuffer* shadowZBuffer
    ) const override;
    std::unique_ptr<ZBuffer> CreateShadowZBuffer(
        int width, int height,
        std::unique_ptr<class ProjectionStrategy> proj
    ) const override;
    bool NeedsShadowMap() const override;
    void MarkShadowMapDirty() override;
    void MarkShadowMapClean() const { shadowMapDirty = false; }
    CameraPtr GetCamera() const override;

    // Getter for light color
    QColor GetColor() const { return color; }
};
