#include "RenderingWidget.h"
#include <QPainter>

RenderingWidget::RenderingWidget(QWidget *parent) : QWidget(parent) {
    setAutoFillBackground(false);  // We handle all painting
    setFocusPolicy(Qt::StrongFocus); // Accept focus to receive mouse clicks that can transfer focus to MainWindow
}

void RenderingWidget::setImage(const QImage &image) {
    m_renderImage = image;
    update(); // Trigger a repaint
}

void RenderingWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    if (!m_renderImage.isNull()) {
        // Scale the image to fit the widget while maintaining aspect ratio
        painter.drawImage(rect(), m_renderImage.scaled(rect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Draw a placeholder if no image is available
        painter.fillRect(rect(), Qt::black);
    }
}

void RenderingWidget::mousePressEvent(QMouseEvent *event) {
    // When clicked, transfer focus to parent window to enable keyboard input
    if (parentWidget()) {
        parentWidget()->setFocus();
    }
    QWidget::mousePressEvent(event);
}