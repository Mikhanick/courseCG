#pragma once
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QtGui/QVector3D>

class LightDirectionControl : public QWidget {
    Q_OBJECT

public:
    explicit LightDirectionControl(QWidget *parent = nullptr);

    QVector3D getDirection() const;
    void setDirection(const QVector3D& direction);

signals:
    void directionChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateControlPointPosition();
    QPointF vectorToWidgetCoordinate(const QVector3D& vec) const;
    QVector3D widgetCoordinateToVector(const QPointF& point) const;
    
    QVector3D m_direction;
    QPointF m_controlPointPos;
    bool m_dragging;
    int m_radius; // Radius of the circular area
};