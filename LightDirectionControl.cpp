#include "LightDirectionControl.h"
#include <QPainter>
#include <QPainterPath>

LightDirectionControl::LightDirectionControl(QWidget *parent)
    : QWidget(parent), m_direction(-0.3f, 0.2f, -0.3f), m_dragging(false) {
    setFixedSize(200, 200); // Square widget
    m_radius = 80; // Circular area radius
    updateControlPointPosition();
}

QVector3D LightDirectionControl::getDirection() const {
    return m_direction;
}

void LightDirectionControl::setDirection(const QVector3D& direction) {
    m_direction = direction.normalized();
    updateControlPointPosition();
    update();
}

void LightDirectionControl::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw circular area
    QPointF centerPoint = rect().center();
    QRectF circleRect(centerPoint.x() - m_radius, centerPoint.y() - m_radius,
                      m_radius * 2, m_radius * 2);

    // Draw grid background
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    for (int i = -2; i <= 2; ++i) {
        // Horizontal lines
        painter.drawLine(circleRect.left(), centerPoint.y() + i * m_radius/2,
                         circleRect.right(), centerPoint.y() + i * m_radius/2);
        // Vertical lines
        painter.drawLine(centerPoint.x() + i * m_radius/2, circleRect.top(),
                         centerPoint.x() + i * m_radius/2, circleRect.bottom());
    }

    // Draw coordinate axes
    painter.setPen(QPen(QColor(150, 150, 200), 2));
    // X-axis (red) - horizontal
    painter.drawLine(circleRect.left(), centerPoint.y(), circleRect.right(), centerPoint.y());
    // Z-axis (blue) - vertical
    painter.drawLine(centerPoint.x(), circleRect.top(), centerPoint.x(), circleRect.bottom());

    // Draw directional vector
    painter.setPen(QPen(QColor(255, 100, 100), 3));
    painter.drawLine(centerPoint, m_controlPointPos);

    // Draw outer circle
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    painter.drawEllipse(centerPoint, m_radius, m_radius);

    // Draw center point (light from top)
    painter.setBrush(QColor(100, 200, 100));
    painter.setPen(QPen(QColor(80, 150, 80), 1));
    painter.drawEllipse(centerPoint, 5, 5);
    
    // Draw control point for light direction
    painter.setBrush(QColor(255, 100, 100));
    painter.setPen(QPen(QColor(200, 80, 80), 1));
    painter.drawEllipse(m_controlPointPos, 8, 8);

    // Draw axis labels
    painter.setPen(QPen(QColor(255, 0, 0), 1));
    painter.drawText(circleRect.right() + 5, centerPoint.y() + 5, "X");
    painter.setPen(QPen(QColor(0, 0, 255), 1));
    painter.drawText(centerPoint.x() - 15, circleRect.top() - 5, "Z");
}

QPointF LightDirectionControl::vectorToWidgetCoordinate(const QVector3D& vec) const {
    // Convert normalized direction vector to widget coordinates
    // X component maps to horizontal position, Z component to vertical
    QPointF centerPoint = rect().center();
    float x = centerPoint.x() + vec.x() * m_radius;
    float y = centerPoint.y() - vec.z() * m_radius; // Invert Z to match widget coordinates
    return QPointF(x, y);
}

QVector3D LightDirectionControl::widgetCoordinateToVector(const QPointF& point) const {
    // Convert widget coordinates to a normalized direction vector
    QPointF centerPoint = rect().center();
    float dx = (point.x() - centerPoint.x()) / m_radius;
    float dz = -(point.y() - centerPoint.y()) / m_radius; // Invert to match coordinate system
    float dy = 0.2f; // Keep Y fixed as specified in the requirements

    // Normalize the vector but maintain Y component
    QVector3D dir(dx, dy, dz);
    float length = sqrt(dx*dx + dy*dy + dz*dz);
    if (length > 0) {
        dir = QVector3D(dx/length, dy/length, dz/length);
    }

    return dir;
}

void LightDirectionControl::updateControlPointPosition() {
    m_controlPointPos = vectorToWidgetCoordinate(m_direction);
}

void LightDirectionControl::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPointF clickPos = event->pos();
        QPointF centerPoint = rect().center();
        float distance = QLineF(clickPos, centerPoint).length();

        if (distance <= m_radius + 10) { // Allow slight tolerance
            m_dragging = true;
            m_controlPointPos = clickPos;
            m_direction = widgetCoordinateToVector(m_controlPointPos);
            update();
        }
    }
}

void LightDirectionControl::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        QPointF mousePos = event->pos();
        QPointF centerPoint = rect().center();
        // Constrain mouse position within the circle
        QLineF lineToCenter(mousePos, centerPoint);
        if (lineToCenter.length() > m_radius) {
            // Limit position to the circle boundary
            lineToCenter.setLength(m_radius);
            mousePos = lineToCenter.p2();
        }

        m_controlPointPos = mousePos;
        m_direction = widgetCoordinateToVector(m_controlPointPos);
        update();
    }
}

void LightDirectionControl::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    if (m_dragging) {
        emit directionChanged();
    }
    m_dragging = false;
}

void LightDirectionControl::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateControlPointPosition();
}