#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QRgb>
#include "LightDirectionControl.h"
#include "ColorControlWidget.h"

class ControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);

    // Getters for control values
    QVector3D getLightDirection() const;
    QVector3D getLightMultipliers() const;  // Returns the multiplier values
    int getMapSize() const;
    unsigned int getSeed() const;
    QPair<int, int> getResolution() const;  // Returns width and height
    float getCameraFOV() const;

signals:
    void lightDirectionChanged(const QVector3D& direction);
    void lightMultipliersChanged(const QVector3D& multipliers);  // Now represents multipliers
    void mapSizeChanged(int size);
    void seedChanged(unsigned int seed);
    void renderQualityChanged(int width, int height);  // Changed to width and height
    void cameraFOVChanged(float fov);
    void regenerateMapRequested();

public slots:
    void updateFPS(double fps);
    void updateRenderTime(double renderTime);

private slots:
    void onLightDirectionChanged();
    void onLightColorChanged();
    void onMapSizeChanged();
    void onSeedChanged();
    void onRenderQualityChanged();
    void onCameraFOVChanged();
    void onRegenerateMapClicked();

private:
    QVBoxLayout *m_mainLayout;

    // FPS and render time display
    QLabel *m_fpsLabel;
    QLabel *m_renderTimeLabel;

    // Light controls
    LightDirectionControl *m_lightDirectionControl;
    ColorControlWidget *m_lightColorControl;

    // Map generation controls
    QSpinBox *m_mapSizeSpinBox;
    QSpinBox *m_seedSpinBox;
    QPushButton *m_regenerateButton;

    // Render resolution and camera FOV
    // Changed to input fields instead of slider for free resolution
    QSpinBox *m_renderWidthSpinBox;
    QSpinBox *m_renderHeightSpinBox;
    QSlider *m_cameraFOVSlider;
    QLabel *m_renderResolutionLabel;
    QLabel *m_cameraFOVLabel;
};