#pragma once
#include <QtGui/QVector3D>
#include <QtGui/QMatrix4x4>
#include <memory>

class Camera {
    QVector3D position;
    QVector3D target;
    QVector3D up;

public:
    Camera(
        const QVector3D& pos = QVector3D(0, 0, 0),
        const QVector3D& tgt = QVector3D(0, 0, -1),
        const QVector3D& u = QVector3D(0, 1, 0)
    ) : position(pos), target(tgt), up(u) {}

    QMatrix4x4 GetViewMatrix() const {
        QMatrix4x4 view;
        view.lookAt(position, target, up);
        return view;
    }

    void SetPosition(const QVector3D& pos) { position = pos; }
    void SetTarget(const QVector3D& tgt) { target = tgt; }
    QVector3D GetPosition() const { return position; }
    QVector3D GetTarget() const { return target; }
    QVector3D GetUp() const { return up; }
};

using CameraPtr = std::shared_ptr<Camera>;