#pragma once
#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include <QtGui/QVector3D>
#include <QtGui/QVector2D>
#include <memory>
#include <chrono>

class Scene;
class Renderer; // ➤ оставляем!
class ControlPanel;
class RenderingWidget;
class QDockWidget;
namespace City {
    class CityMap;
}

#include "GlobalRandom.h"  // Include global random declarations

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void OnTimeout();
    void OnLightDirectionChanged(const QVector3D& direction);
    void OnLightMultipliersChanged(const QVector3D& multipliers);
    void OnMapSizeChanged(int size);
    void OnSeedChanged(unsigned int seed);
    void OnResolutionChanged(int width, int height);
    void OnCameraFOVChanged(float fov);
    void OnRegenerateMapRequested();

private:
    void SetupUI();
    void GenerateCity(int gridSize = 5);
    void GenerateCityWithMap(); // New method to generate city using CityMap
    void RenderScene(); // вызывает m_renderer->Render(*m_scene, m_image)
    void UpdateCamera();
    void UpdateLightDirection(const QVector3D& direction);
    void UpdateLightColor(QRgb color);  // Keep this for compatibility if needed
    void UpdateLightMultipliers(const QVector3D& multipliers);
    void RegenerateScene();
    void applyPendingResolutionChange();
    void EnsureImageBuffersInitialized();

    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Renderer> m_renderer; // ➤ оставляем!
    std::unique_ptr<City::CityMap> m_cityMap; // New city map
    QImage m_renderImage;  // Changed name to be more specific
    RenderingWidget *m_renderingWidget;  // New rendering widget
    QDockWidget *m_controlDock;
    ControlPanel *m_controlPanel;

    // Timing for FPS calculation
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    int m_frameCount = 0;
    double m_fps = 0.0;
    double m_renderTime = 0.0; // Time taken to render the last frame

    // Additional timing variables
    double m_actualRenderTime = 0.0; // Renamed from some other variable

    // Render parameters
    int m_renderWidth = 1920; // Free resolution width
    int m_renderHeight = 1080; // Free resolution height
    float m_cameraFOV = 90.0f;

    // Resolution change handling
    bool m_resolutionChangePending = false;
    int m_pendingRenderWidth = 1920;
    int m_pendingRenderHeight = 1080;

    // Map generation parameters
    int m_mapSize = 1000000;
    unsigned int m_currentSeed = 5;

    // Управление камерой (без изменений)
    QVector3D m_cameraPos;
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    QVector2D m_lastMousePos;
    bool m_mousePressed = false;
    bool m_keyW = false, m_keyS = false, m_keyA = false, m_keyD = false;
    bool m_keyShift = false; // New shift key for speed boost
    float m_moveSpeed = 0.1f;
    float m_rotateSpeed = 0.5f;

    QTimer m_timer;
};
