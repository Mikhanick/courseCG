#pragma once
#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include <QtGui/QVector3D>
#include <QtGui/QVector2D>
#include <memory>

class Scene;
class Renderer; // ➤ оставляем!
namespace City {
    class CityMap;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void OnTimeout();

private:
    void GenerateCity(int gridSize = 5);
    void GenerateCityWithMap(); // New method to generate city using CityMap
    void RenderScene(); // вызывает m_renderer->Render(*m_scene, m_image)
    void UpdateCamera();

    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Renderer> m_renderer; // ➤ оставляем!
    std::unique_ptr<City::CityMap> m_cityMap; // New city map
    QImage m_image;

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
