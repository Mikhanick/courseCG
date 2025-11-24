#pragma once
#include <QWidget>
#include <QImage>
#include <QMouseEvent>

class RenderingWidget : public QWidget {
    Q_OBJECT

public:
    explicit RenderingWidget(QWidget *parent = nullptr);

    void setImage(const QImage &image);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QImage m_renderImage;
};