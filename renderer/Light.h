#pragma once
#include <QtGui/QVector3D>
#include <memory>
#include "ZBuffer.h"

class Camera;
using CameraPtr = std::shared_ptr<Camera>;

class Light {
public:
    virtual ~Light() = default;
    virtual QVector3D GetDirectionTo(const QVector3D& point) const = 0;
    virtual float ComputeShadowFactor(
        const QVector3D& worldPos,
        const ZBuffer* shadowZBuffer,
        const Camera* lightCamera
    ) const = 0;
    virtual std::unique_ptr<ZBuffer> CreateShadowZBuffer(
        int width, int height,
        std::unique_ptr<class ProjectionStrategy> proj
    ) const = 0;
    virtual bool NeedsShadowMap() const = 0;
    virtual void MarkShadowMapDirty() = 0;
    virtual CameraPtr GetCamera() const = 0;
};

