#include "DirectionalLight.h"
#include "Camera.h"
#include "ProjectionStrategy.h"

DirectionalLight::DirectionalLight(const QVector3D& dir, const QColor& lightColor)
    : direction(dir.normalized()), color(lightColor) {}

void DirectionalLight::SetDirection(const QVector3D& dir) {
    QVector3D newDir = dir.normalized();
    if (newDir != direction) {
        direction = newDir;
        MarkShadowMapDirty();
    }
}

QVector3D DirectionalLight::GetDirectionTo(const QVector3D& point) const {
    return -direction; // направление от точки к свету
}

float DirectionalLight::ComputeShadowFactor(
    const QVector3D& worldPos,
    const ZBuffer* shadowZBuffer
    ) const {
    if (!shadowZBuffer) return 1.0f;

    float x, y, depth, w;  // ← ДОБАВЛЕНО: w
    // Передаём 4 параметра в Project
    if (!shadowZBuffer->Project(worldPos, x, y, depth, w) || !shadowZBuffer->InBounds(x, y)) {
        return 1.0f;
    }

    float storedDepth = shadowZBuffer->At(static_cast<int>(x), static_cast<int>(y));
    const float bias = 0.00003f;

    if (depth > storedDepth + bias) {
        return 0.4f; // тень = 0.4f
    }
    return 1.0f;
}

std::unique_ptr<ZBuffer> DirectionalLight::CreateShadowZBuffer(
    int width, int height,
    std::unique_ptr<ProjectionStrategy> proj
    ) const {
    return std::make_unique<ZBuffer>(width, height, GetCamera(), std::move(proj));
}

bool DirectionalLight::NeedsShadowMap() const {
    return shadowMapDirty;
}

void DirectionalLight::MarkShadowMapDirty() {
    shadowMapDirty = true;
}

CameraPtr DirectionalLight::GetCamera() const {
    if (!cachedCamera) {
        // Создаём камеру вдоль направления света
        QVector3D pos = direction * 4000.;
        cachedCamera = std::make_shared<Camera>(pos, QVector3D(0,0,0), QVector3D(0,1,0));
    }
    return cachedCamera;
}
