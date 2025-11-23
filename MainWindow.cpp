#include "MainWindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>
#include "renderer/Scene.h"
#include "renderer/Renderer.h"
#include "renderer/Camera.h"
#include "renderer/DirectionalLight.h"
#include "renderer/GraphicObject.h"
#include "city/core/CityMap.h"
#include "city/strategies/SimpleRoadGenerationStrategy.h"
#include <QDebug>
#include "city/strategies/SimpleBuildingSelector.h"
#include "city/strategies/SubdivisionRoadGenerationStrategy.h"
#include "city/strategies/SmartBuildingSelector.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

// Include for global random functionality
#include "GlobalRandom.h"

// Global random seed variable definition
unsigned int randomSeed = 5; // Default seed value

// Global random number generator definition
std::mt19937 globalRandomGenerator(randomSeed);

// Function to update random generators with new seed
void updateRandomGenerators(unsigned int newSeed) {
    randomSeed = newSeed;
    globalRandomGenerator.seed(randomSeed); // Re-seed the global generator
}

using City::CityMap;
using City::SimpleBuildingSelector;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_image(1920, 1200, QImage::Format_RGB32) {
    setFixedSize(1920, 1200);
    setWindowTitle("Software Renderer - City Generator");

    m_cameraPos = QVector3D(40, 15, -11);
    m_yaw = 220.0f;
    m_pitch = -15.0f;

    // Increase base movement speed by 9x total (3x previous increase + another 3x)
    m_moveSpeed = 3.5f; // Was 0.3f (0.1f * 3 * 3)
    m_rotateSpeed = 0.5f;

    m_scene = std::make_unique<Scene>();
    m_scene->camera = std::make_shared<Camera>(m_cameraPos, QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    m_renderer = std::make_unique<Renderer>(1920, 1200); // ➤ инициализируем рендерер

    m_scene->AddLight(new DirectionalLight(QVector3D(-0.3, 0.2, -0.3)));
    GenerateCityWithMap(); // Use the new city map generation
    m_renderer->UpdateShadowBuffers(*m_scene.get());
    connect(&m_timer, &QTimer::timeout, this, &MainWindow::OnTimeout);
    m_timer.start(16);
}

MainWindow::~MainWindow() = default;
void MainWindow::GenerateCity(int gridSize) {
    float spacing = 3.0f;
    float minHeight = 1.0f;
    float maxHeight = 6.0f;

    // Создание земли — зелёный квадрат под весь город
    auto ground = new GraphicObject();

    // Определяем границы земли: от -gridSize до +gridSize с учётом spacing
    float groundSize = gridSize * spacing;

    // Вершины земли (квадрат в плоскости Y=0)
    ground->AddPoint(QVector3D(-groundSize - spacing/2, 0, -groundSize - spacing/2)); // 0: левый-задний
    ground->AddPoint(QVector3D( groundSize + spacing/2, 0, -groundSize - spacing/2)); // 1: правый-задний
    ground->AddPoint(QVector3D( groundSize + spacing/2, 0,  groundSize + spacing/2)); // 2: правый-передний
    ground->AddPoint(QVector3D(-groundSize - spacing/2, 0,  groundSize + spacing/2)); // 3: левый-передний

    // Цвет земли — зелёный
    QColor greenColor(34, 139, 34); // ForestGreen

    // Два полигона: верх (лицевой) и низ (обратный, для полноты)
    // Верхняя грань (лицевая, против часовой стрелки)
    ground->AddFace(0, 1, 2, greenColor);
    ground->AddFace(0, 2, 3, greenColor);

    // Нижняя грань (если нужно — по часовой, чтобы нормаль смотрела вниз)
    ground->AddFace(0, 3, 2, greenColor);
    ground->AddFace(0, 2, 1, greenColor);

    ground->ComputeFaceNormals();
    m_scene->AddObject(ground);

    // Генерация зданий
    for (int i = -gridSize; i <= gridSize; ++i) {
        for (int j = -gridSize; j <= gridSize; ++j) {
            if ((i + j) % 2 == 0) { // шахматный порядок
                float height = minHeight + (maxHeight - minHeight) * ((i * i + j * j) % 5) / 4.0f;

                auto building = new GraphicObject();

                // Вершины здания (прямоугольная призма)
                float x = i * spacing;
                float z = j * spacing;
                building->AddPoint(QVector3D(x - 0.5f, 0, z - 0.5f)); // 0
                building->AddPoint(QVector3D(x + 0.5f, 0, z - 0.5f)); // 1
                building->AddPoint(QVector3D(x + 0.5f, height, z - 0.5f)); // 2
                building->AddPoint(QVector3D(x - 0.5f, height, z - 0.5f)); // 3
                building->AddPoint(QVector3D(x - 0.5f, 0, z + 0.5f)); // 4
                building->AddPoint(QVector3D(x + 0.5f, 0, z + 0.5f)); // 5
                building->AddPoint(QVector3D(x + 0.5f, height, z + 0.5f)); // 6
                building->AddPoint(QVector3D(x - 0.5f, height, z + 0.5f)); // 7

                // Цвет зданий
                int clr = std::max(120, (std::abs(i) * 10 + std::abs(j) * 10 + 100) % 230);
                QColor color(clr, clr, clr + 20);
                QColor roof(60, 60, 60);
                // Грани здания
                building->AddFace(0, 2, 1, color); building->AddFace(2, 0, 3, color); // перед
                building->AddFace(1, 6, 5, color); building->AddFace(6, 1, 2, color); // право
                building->AddFace(5, 7, 4, color); building->AddFace(7, 5, 6, color); // зад
                building->AddFace(4, 3, 0, color); building->AddFace(3, 4, 7, color); // лево
                building->AddFace(3, 6, 2, roof); building->AddFace(6, 3, 7, roof); // верх

                building->ComputeFaceNormals();
                m_scene->AddObject(building);
            }
        }
    }
}

void MainWindow::GenerateCityWithMap() {
    // Создание генератора города с использованием новых классов
    auto roadGen = std::make_unique<City::SubdivisionRoadGenerationStrategy>();
    auto buildingSelector = std::make_unique<City::SmartBuildingSelector>("/drive_d/Documents/CG_curs/program/buildings");
    // auto buildingSelector = std::make_unique<City::SimpleBuildingSelector>();

    m_cityMap = std::make_unique<City::CityMap>(
        std::move(roadGen),
        std::move(buildingSelector)
    );

    m_cityMap->generate(1000000.0f);
    qDebug() << "Передано в отрисовку";
    // Добавление объектов из карты города в сцену
    auto objects = m_cityMap->exportToScene();

    // Calculate the extent of the city to determine ground size
    float minX = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    // Calculate bounds for all city objects
    for (const auto& cityObj : objects) {
        for (const auto& point : cityObj.points) {
            minX = std::min(minX, point.x());
            minZ = std::min(minZ, point.z());
            maxX = std::max(maxX, point.x());
            maxZ = std::max(maxZ, point.z());
        }
    }

    // If no objects were found, use a default size
    if (objects.empty()) {
        minX = minZ = -50.0f;
        maxX = maxZ = 50.0f;
    } else {
        // Add some margin around the city
        const float margin = 20.0f;
        minX -= margin;
        maxX += margin;
        minZ -= margin;
        maxZ += margin;
    }

    // Создание земли — зелёный квадрат под весь город, состоящий из меньших полигонов
    const float tileSize = 10.0f; // Размер одного тайла земли

    // Создаем сетку из меньших квадратов вместо одного большого полигона
    for (float x = minX; x < maxX; x += tileSize) {
        for (float z = minZ; z < maxZ; z += tileSize) {
            auto tile = new GraphicObject();
            float maxX_tile = qMin(x + tileSize, maxX);
            float maxZ_tile = qMin(z + tileSize, maxZ);

            // Вершины тайла (квадрат в плоскости Y=0)
            tile->AddPoint(QVector3D(x, 0, z));           // 0: левый-задний
            tile->AddPoint(QVector3D(maxX_tile, 0, z));   // 1: правый-задний
            tile->AddPoint(QVector3D(maxX_tile, 0, maxZ_tile)); // 2: правый-передний
            tile->AddPoint(QVector3D(x, 0, maxZ_tile));   // 3: левый-передний

            // Цвет земли — зелёный
            // QColor greenColor(235, 245, 255); // Snow
            QColor greenColor(34, 139, 34); // ForestGreen

            // Грани тайла (исправленный порядок для правильного отображения сверху)
            tile->AddFace(0, 2, 1, greenColor); // Изменен порядок точек для правильной нормали
            tile->AddFace(0, 3, 2, greenColor); // Изменен порядок точек для правильной нормали

            tile->ComputeFaceNormals();
            m_scene->AddObject(tile);
        }
    }

    // Now add the actual city objects to the main scene
    for (auto& cityObj : objects) {
        auto* newObj = new GraphicObject();
        newObj->points = std::move(cityObj.points);
        newObj->faces = std::move(cityObj.faces);
        m_scene->AddObject(newObj);
    }
}

void MainWindow::RenderScene() {
    m_renderer->Render(*m_scene, m_image); // ➤ вызываем метод рендерера
    update();
}

void MainWindow::UpdateCamera() {
    // Вычисляем направление взгляда
    QMatrix4x4 rotation;
    rotation.rotate(m_pitch, QVector3D(1, 0, 0));
    rotation.rotate(m_yaw, QVector3D(0, 1, 0));

    QVector3D front = rotation.map(QVector3D(0, 0, -1));
    front.normalize();

    // Apply speed boost when Shift is pressed
    float currentSpeed = m_moveSpeed;
    if (m_keyShift) {
        currentSpeed *= 3.0f; // Triple speed when Shift is pressed (increased from 2x to 3x)
    }

    m_cameraPos += (m_keyW - m_keyS) * currentSpeed * front;
    QVector3D right = QVector3D::crossProduct(front, QVector3D(0, 1, 0)).normalized();
    m_cameraPos += (m_keyD - m_keyA) * currentSpeed * right;

    // Обновляем камеру в сцене
    m_scene->camera->SetPosition(m_cameraPos);
    m_scene->camera->SetTarget(m_cameraPos + front);
}

void MainWindow::OnTimeout() {
    UpdateCamera();
    RenderScene();
}

void MainWindow::paintEvent(QPaintEvent* /*event*/) { // ✅ игнорируем
    QPainter painter(this);
    painter.drawImage(rect(), m_image);
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = QVector2D(event->pos());
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_mousePressed) {
        QVector2D currentPos(event->pos());
        QVector2D delta = currentPos - m_lastMousePos;

        m_yaw += delta.x() * m_rotateSpeed;
        m_pitch -= delta.y() * m_rotateSpeed;

        // Ограничение pitch
        m_pitch = qBound(-89.0f, m_pitch, 89.0f);

        m_lastMousePos = currentPos;
    }
}

void MainWindow::wheelEvent(QWheelEvent* event) {
    m_moveSpeed += event->angleDelta().y() * 0.001f;
    m_moveSpeed = qBound(0.05f, m_moveSpeed, 3.5f);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) m_keyW = true;
    if (event->key() == Qt::Key_S) m_keyS = true;
    if (event->key() == Qt::Key_A) m_keyA = true;
    if (event->key() == Qt::Key_D) m_keyD = true;
    if (event->key() == Qt::Key_Shift) m_keyShift = true;
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) m_keyW = false;
    if (event->key() == Qt::Key_S) m_keyS = false;
    if (event->key() == Qt::Key_A) m_keyA = false;
    if (event->key() == Qt::Key_D) m_keyD = false;
    if (event->key() == Qt::Key_Shift) m_keyShift = false;
}
