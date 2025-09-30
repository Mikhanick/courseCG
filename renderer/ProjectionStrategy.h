#pragma once
#include <QtGui/QVector3D>
#include <memory>
#include "Camera.h"

class ProjectionStrategy {
public:
    virtual ~ProjectionStrategy() = default;
    virtual bool Project(const QVector3D& worldPos, const Camera& camera, float& x, float& y, float& z, float& w) const = 0;
    virtual std::unique_ptr<ProjectionStrategy> Clone() const = 0;
};

class OrthographicProjection : public ProjectionStrategy {
    float left, right, bottom, top, nearPlane, farPlane;

public:
    OrthographicProjection(
        float l = -50, float r = 50,
        float b = -50, float t = 50,
        float n = 0.1f, float f = 100.0f
        ) : left(l), right(r), bottom(b), top(t), nearPlane(n), farPlane(f) {}

    bool Project(const QVector3D& worldPos, const Camera& camera, float& x, float& y, float& z, float& w) const override {
        QMatrix4x4 view = camera.GetViewMatrix();
        QVector3D viewPos = view.map(worldPos);

        if (viewPos.z() < -farPlane || viewPos.z() > -nearPlane) {
            return false;
        }

        QMatrix4x4 proj;
        proj.ortho(left, right, bottom, top, nearPlane, farPlane);
        QVector4D clip = proj * QVector4D(viewPos, 1.0f);

        if (clip.w() == 0) return false;

        x = clip.x();
        y = clip.y();
        z = clip.z();
        w = clip.w();

        return true;
    }

    std::unique_ptr<ProjectionStrategy> Clone() const override {
        return std::make_unique<OrthographicProjection>(left, right, bottom, top, nearPlane, farPlane);
    }
};

class PerspectiveProjection : public ProjectionStrategy {
    float fov, aspect, nearPlane, farPlane;

public:
    PerspectiveProjection(
        float f = 60.0f, float a = 1.0f,
        float n = 0.1f, float far = 100.0f
        ) : fov(f), aspect(a), nearPlane(n), farPlane(far) {}

    bool Project(const QVector3D& worldPos, const Camera& camera, float& x, float& y, float& z, float& w) const override {
        QMatrix4x4 view = camera.GetViewMatrix();
        QVector3D viewPos = view.map(worldPos);

        QMatrix4x4 proj;
        proj.perspective(fov, aspect, nearPlane, farPlane);
        QVector4D clip = proj * QVector4D(viewPos, 1.0f);

        if (clip.w() == 0) return false;

        x = clip.x();
        y = clip.y();
        z = clip.z();
        w = clip.w();

        return true;
    }

    std::unique_ptr<ProjectionStrategy> Clone() const override {
        return std::make_unique<PerspectiveProjection>(fov, aspect, nearPlane, farPlane);
    }
};
