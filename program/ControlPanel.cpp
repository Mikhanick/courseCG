#include "ControlPanel.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGridLayout>

ControlPanel::ControlPanel(QWidget *parent) : QWidget(parent) {
    m_mainLayout = new QVBoxLayout(this);

    // FPS and render time display
    auto *infoLayout = new QHBoxLayout();
    m_fpsLabel = new QLabel("FPS: 0");
    m_renderTimeLabel = new QLabel("Время рендеринга: 0мс");
    infoLayout->addWidget(m_fpsLabel);
    infoLayout->addWidget(m_renderTimeLabel);
    infoLayout->addStretch(); // Add stretch to align labels to the left

    m_mainLayout->addLayout(infoLayout);

    // Light controls group
    auto *lightGroup = new QGroupBox("Управление светом");
    auto *lightLayout = new QHBoxLayout();

    // Light direction control
    m_lightDirectionControl = new LightDirectionControl();
    connect(m_lightDirectionControl, &LightDirectionControl::directionChanged,
            this, &ControlPanel::onLightDirectionChanged);

    // Light color control
    m_lightColorControl = new ColorControlWidget();
    connect(m_lightColorControl, &ColorControlWidget::multipliersChanged,
            this, &ControlPanel::onLightColorChanged);

    lightLayout->addWidget(m_lightDirectionControl);
    lightLayout->addWidget(m_lightColorControl);
    lightGroup->setLayout(lightLayout);

    m_mainLayout->addWidget(lightGroup);

    // Map generation controls group
    auto *mapGroup = new QGroupBox("Генерация карты");
    auto *mapLayout = new QFormLayout();

    m_mapSizeSpinBox = new QSpinBox();
    m_mapSizeSpinBox->setRange(50, 1000000);
    m_mapSizeSpinBox->setValue(1000000);
    m_mapSizeSpinBox->setSuffix(" ед.");
    connect(m_mapSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlPanel::onMapSizeChanged);

    m_seedSpinBox = new QSpinBox();
    m_seedSpinBox->setRange(0, 999999);
    m_seedSpinBox->setValue(5);
    connect(m_seedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlPanel::onSeedChanged);

    m_regenerateButton = new QPushButton("Сгенерировать карту");
    connect(m_regenerateButton, &QPushButton::clicked,
            this, &ControlPanel::onRegenerateMapClicked);

    mapLayout->addRow("Размер:", m_mapSizeSpinBox);
    mapLayout->addRow("Зерно:", m_seedSpinBox);
    mapLayout->addRow(m_regenerateButton);
    mapGroup->setLayout(mapLayout);

    m_mainLayout->addWidget(mapGroup);

    // Render settings group
    auto *renderGroup = new QGroupBox("Настройки рендеринга");
    auto *renderLayout = new QFormLayout();

    // Resolution controls (width and height as spin boxes)
    m_renderWidthSpinBox = new QSpinBox();
    m_renderWidthSpinBox->setRange(100, 10000); // Reasonable range for width
    m_renderWidthSpinBox->setValue(1920);
    connect(m_renderWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlPanel::onRenderQualityChanged);

    m_renderHeightSpinBox = new QSpinBox();
    m_renderHeightSpinBox->setRange(100, 10000); // Reasonable range for height
    m_renderHeightSpinBox->setValue(1080);
    connect(m_renderHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlPanel::onRenderQualityChanged);

    auto *resolutionLayout = new QHBoxLayout();
    resolutionLayout->addWidget(m_renderWidthSpinBox);
    resolutionLayout->addWidget(new QLabel("x"));
    resolutionLayout->addWidget(m_renderHeightSpinBox);

    m_renderResolutionLabel = new QLabel("1920x1080");
    connect(m_renderWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int width) {
        m_renderResolutionLabel->setText(QString("%1x%2").arg(width).arg(m_renderHeightSpinBox->value()));
    });
    connect(m_renderHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int height) {
        m_renderResolutionLabel->setText(QString("%1x%2").arg(m_renderWidthSpinBox->value()).arg(height));
    });

    m_cameraFOVSlider = new QSlider(Qt::Horizontal);
    m_cameraFOVSlider->setRange(10, 120);  // Changed minimum FOV to 10 as requested
    m_cameraFOVSlider->setValue(90);
    // Only emit on slider release, not during dragging
    connect(m_cameraFOVSlider, &QSlider::sliderReleased,
            this, &ControlPanel::onCameraFOVChanged);

    m_cameraFOVLabel = new QLabel("90°");
    connect(m_cameraFOVSlider, &QSlider::valueChanged, [this](int value) {
        m_cameraFOVLabel->setText(QString("%1°").arg(value));
    });

    auto *fovLayout = new QHBoxLayout();
    fovLayout->addWidget(m_cameraFOVSlider);
    fovLayout->addWidget(m_cameraFOVLabel);

    renderLayout->addRow("Разрешение:", resolutionLayout);
    renderLayout->addRow(m_renderResolutionLabel);
    renderLayout->addRow("Угол обзора:", fovLayout);
    renderGroup->setLayout(renderLayout);

    m_mainLayout->addWidget(renderGroup);

    m_mainLayout->addStretch(); // Add stretch to use remaining space
}

QVector3D ControlPanel::getLightDirection() const {
    return m_lightDirectionControl->getDirection();
}

QVector3D ControlPanel::getLightMultipliers() const {
    return m_lightColorControl->getMultipliers();
}

int ControlPanel::getMapSize() const {
    return m_mapSizeSpinBox->value();
}

unsigned int ControlPanel::getSeed() const {
    return static_cast<unsigned int>(m_seedSpinBox->value());
}

QPair<int, int> ControlPanel::getResolution() const {
    return qMakePair(m_renderWidthSpinBox->value(), m_renderHeightSpinBox->value());
}

float ControlPanel::getCameraFOV() const {
    return static_cast<float>(m_cameraFOVSlider->value());
}

void ControlPanel::setLightDirection(const QVector3D& direction) {
    m_lightDirectionControl->setDirection(direction);
}

void ControlPanel::setMultipliers(const QVector3D& multipliers) {
    m_lightColorControl->setMultipliers(multipliers);
}

void ControlPanel::setResolution(const QPair<int, int>& resolution) {
    m_renderWidthSpinBox->setValue(resolution.first);
    m_renderHeightSpinBox->setValue(resolution.second);
}

void ControlPanel::setCameraFOV(float fov) {
    m_cameraFOVSlider->setValue(static_cast<int>(fov));
}

void ControlPanel::onLightDirectionChanged() {
    emit lightDirectionChanged(getLightDirection());
}

void ControlPanel::onLightColorChanged() {
    emit lightMultipliersChanged(getLightMultipliers());
}

void ControlPanel::onMapSizeChanged() {
    emit mapSizeChanged(getMapSize());
}

void ControlPanel::onSeedChanged() {
    emit seedChanged(getSeed());
}

void ControlPanel::onRenderQualityChanged() {
    auto resolution = getResolution();
    emit renderQualityChanged(resolution.first, resolution.second);
}

void ControlPanel::onCameraFOVChanged() {
    emit cameraFOVChanged(getCameraFOV());
}

void ControlPanel::onRegenerateMapClicked() {
    emit regenerateMapRequested();
}

void ControlPanel::updateFPS(double fps) {
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}

void ControlPanel::updateRenderTime(double renderTime) {
    m_renderTimeLabel->setText(QString("Затрачено: %1ms").arg(renderTime, 0, 'f', 2));
}
