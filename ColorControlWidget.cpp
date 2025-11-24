#include "ColorControlWidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QDoubleSpinBox>

ColorControlWidget::ColorControlWidget(QWidget *parent)
    : QWidget(parent), m_redMultiplier(1.0), m_greenMultiplier(1.0), m_blueMultiplier(1.0) {

    m_mainLayout = new QVBoxLayout(this);

    // Create labels
    m_redLabel = new QLabel("R:");
    m_greenLabel = new QLabel("G:");
    m_blueLabel = new QLabel("B:");

    // Create spin boxes for direct float multiplier input
    m_redSpinBox = new QDoubleSpinBox();
    m_redSpinBox->setRange(0.0, 10.0);  // Allow multipliers up to 10.0
    m_redSpinBox->setDecimals(3);       // 3 decimal places for precision
    m_redSpinBox->setValue(1.3);        // Initial value from the formula: * 1.3
    m_redSpinBox->setSuffix("x");
    // Connect to editingFinished instead of valueChanged to only emit when editing is complete
    connect(m_redSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ColorControlWidget::onRedSpinBoxChanged);
    connect(m_redSpinBox, &QDoubleSpinBox::editingFinished,
            this, &ColorControlWidget::onRedSpinBoxEditingFinished);

    m_greenSpinBox = new QDoubleSpinBox();
    m_greenSpinBox->setRange(0.0, 10.0);
    m_greenSpinBox->setDecimals(3);
    m_greenSpinBox->setValue(1.1);      // Initial value from the formula: * 1.1
    m_greenSpinBox->setSuffix("x");
    connect(m_greenSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ColorControlWidget::onGreenSpinBoxChanged);
    connect(m_greenSpinBox, &QDoubleSpinBox::editingFinished,
            this, &ColorControlWidget::onGreenSpinBoxEditingFinished);

    m_blueSpinBox = new QDoubleSpinBox();
    m_blueSpinBox->setRange(0.0, 10.0);
    m_blueSpinBox->setDecimals(3);
    m_blueSpinBox->setValue(1.0);       // Initial value from the formula: * 1.0
    m_blueSpinBox->setSuffix("x");
    connect(m_blueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ColorControlWidget::onBlueSpinBoxChanged);
    connect(m_blueSpinBox, &QDoubleSpinBox::editingFinished,
            this, &ColorControlWidget::onBlueSpinBoxEditingFinished);

    // Layout for controls
    auto *redLayout = new QHBoxLayout();
    redLayout->addWidget(m_redLabel);
    redLayout->addWidget(m_redSpinBox);

    auto *greenLayout = new QHBoxLayout();
    greenLayout->addWidget(m_greenLabel);
    greenLayout->addWidget(m_greenSpinBox);

    auto *blueLayout = new QHBoxLayout();
    blueLayout->addWidget(m_blueLabel);
    blueLayout->addWidget(m_blueSpinBox);

    m_mainLayout->addLayout(redLayout);
    m_mainLayout->addLayout(greenLayout);
    m_mainLayout->addLayout(blueLayout);

    // Preview label (for visual feedback)
    m_previewLabel = new QLabel();
    m_previewLabel->setFixedHeight(40);
    m_previewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    updatePreviewColor();
    m_mainLayout->addWidget(m_previewLabel);

    setFixedWidth(180); // Fixed width for consistency
}

QVector3D ColorControlWidget::getMultipliers() const {
    return QVector3D(m_redMultiplier, m_greenMultiplier, m_blueMultiplier);
}

void ColorControlWidget::setMultipliers(const QVector3D& multipliers) {
    m_redMultiplier = multipliers.x();
    m_greenMultiplier = multipliers.y();
    m_blueMultiplier = multipliers.z();

    m_redSpinBox->setValue(m_redMultiplier);
    m_greenSpinBox->setValue(m_greenMultiplier);
    m_blueSpinBox->setValue(m_blueMultiplier);

    updatePreviewColor();
}

void ColorControlWidget::onRedSpinBoxChanged(double value) {
    m_redMultiplier = value;
    updatePreviewColor();
    // Only emit during dragging for preview - not for final change
}

void ColorControlWidget::onGreenSpinBoxChanged(double value) {
    m_greenMultiplier = value;
    updatePreviewColor();
    // Only emit during dragging for preview - not for final change
}

void ColorControlWidget::onBlueSpinBoxChanged(double value) {
    m_blueMultiplier = value;
    updatePreviewColor();
    // Only emit during dragging for preview - not for final change
}

void ColorControlWidget::onRedSpinBoxEditingFinished() {
    emit multipliersChanged();
}

void ColorControlWidget::onGreenSpinBoxEditingFinished() {
    emit multipliersChanged();
}

void ColorControlWidget::onBlueSpinBoxEditingFinished() {
    emit multipliersChanged();
}

void ColorControlWidget::updatePreviewColor() {
    // Calculate an average color for preview based on multipliers
    // Using normalized multipliers to create a visual color representation
    int r = qBound(0, static_cast<int>(qMin(m_redMultiplier * 255.0, 255.0)), 255);
    int g = qBound(0, static_cast<int>(qMin(m_greenMultiplier * 255.0, 255.0)), 255);
    int b = qBound(0, static_cast<int>(qMin(m_blueMultiplier * 255.0, 255.0)), 255);

    QColor color(r, g, b);
    QPixmap pixmap(160, 40);  // Wider pixmap to show the color
    pixmap.fill(color);
    m_previewLabel->setPixmap(pixmap);
}