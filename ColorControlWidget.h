#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QtGui/QVector3D>
#include <QRgb>

class ColorControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ColorControlWidget(QWidget *parent = nullptr);

    // Returns the multiplier values as floats
    QVector3D getMultipliers() const;
    void setMultipliers(const QVector3D& multipliers);

signals:
    void multipliersChanged();

private slots:
    void onRedSpinBoxChanged(double value);
    void onGreenSpinBoxChanged(double value);
    void onBlueSpinBoxChanged(double value);
    void onRedSpinBoxEditingFinished();
    void onGreenSpinBoxEditingFinished();
    void onBlueSpinBoxEditingFinished();

private:
    void updatePreviewColor();

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlsLayout;
    QDoubleSpinBox *m_redSpinBox;
    QDoubleSpinBox *m_greenSpinBox;
    QDoubleSpinBox *m_blueSpinBox;
    QLabel *m_redLabel;
    QLabel *m_greenLabel;
    QLabel *m_blueLabel;
    QLabel *m_previewLabel;

    double m_redMultiplier;
    double m_greenMultiplier;
    double m_blueMultiplier;
};