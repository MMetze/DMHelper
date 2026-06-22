#include "layerparticlesettings.h"
#include "ui_layerparticlesettings.h"
#include "colorpushbutton.h"

LayerParticleSettings::LayerParticleSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerParticleSettings)
{
    ui->setupUi(this);

    ui->btnRainColor->setColor(QColor(200, 200, 255, 180));
    ui->btnRainColor->setRotationVisible(false);

    connect(ui->spinCount, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::particleCountChanged);
    connect(ui->slideCount, &QSlider::valueChanged, ui->spinCount, &QSpinBox::setValue);
    connect(ui->spinCount, qOverload<int>(&QSpinBox::valueChanged), ui->slideCount, &QSlider::setValue);

    connect(ui->spinSpeed, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainSpeedChanged);
    connect(ui->slideSpeed, &QSlider::valueChanged, ui->spinSpeed, &QSpinBox::setValue);
    connect(ui->spinSpeed, qOverload<int>(&QSpinBox::valueChanged), ui->slideSpeed, &QSlider::setValue);

    connect(ui->spinDirection, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainDirectionChanged);
    connect(ui->slideDirection, &QSlider::valueChanged, ui->spinDirection, &QSpinBox::setValue);
    connect(ui->spinDirection, qOverload<int>(&QSpinBox::valueChanged), ui->slideDirection, &QSlider::setValue);

    connect(ui->spinAngle, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainAngleChanged);
    connect(ui->slideAngle, &QSlider::valueChanged, ui->spinAngle, &QSpinBox::setValue);
    connect(ui->spinAngle, qOverload<int>(&QSpinBox::valueChanged), ui->slideAngle, &QSlider::setValue);

    connect(ui->spinLength, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainLengthChanged);
    connect(ui->slideLength, &QSlider::valueChanged, ui->spinLength, &QSpinBox::setValue);
    connect(ui->spinLength, qOverload<int>(&QSpinBox::valueChanged), ui->slideLength, &QSlider::setValue);

    connect(ui->spinOpacity, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainOpacityChanged);
    connect(ui->slideOpacity, &QSlider::valueChanged, ui->spinOpacity, &QSpinBox::setValue);
    connect(ui->spinOpacity, qOverload<int>(&QSpinBox::valueChanged), ui->slideOpacity, &QSlider::setValue);

    connect(ui->btnRainColor, &ColorPushButton::colorChanged, this, &LayerParticleSettings::rainColorChanged);

    connect(ui->spinWidth, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainWidthChanged);
    connect(ui->slideWidth, &QSlider::valueChanged, ui->spinWidth, &QSpinBox::setValue);
    connect(ui->spinWidth, qOverload<int>(&QSpinBox::valueChanged), ui->slideWidth, &QSlider::setValue);

    connect(ui->spinMovement, qOverload<int>(&QSpinBox::valueChanged), this, &LayerParticleSettings::rainMovementChanged);
    connect(ui->slideMovement, &QSlider::valueChanged, ui->spinMovement, &QSpinBox::setValue);
    connect(ui->spinMovement, qOverload<int>(&QSpinBox::valueChanged), ui->slideMovement, &QSlider::setValue);

    connect(ui->comboPreset, qOverload<int>(&QComboBox::activated), this, &LayerParticleSettings::applyPreset);
}

LayerParticleSettings::~LayerParticleSettings()
{
    delete ui;
}

int LayerParticleSettings::particleCount() const
{
    return ui->spinCount->value();
}

int LayerParticleSettings::rainSpeed() const
{
    return ui->spinSpeed->value();
}

int LayerParticleSettings::rainDirection() const
{
    return ui->spinDirection->value();
}

int LayerParticleSettings::rainAngle() const
{
    return ui->spinAngle->value();
}

QColor LayerParticleSettings::rainColor() const
{
    return ui->btnRainColor->getColor();
}

int LayerParticleSettings::rainLength() const
{
    return ui->spinLength->value();
}

int LayerParticleSettings::rainOpacity() const
{
    return ui->spinOpacity->value();
}

int LayerParticleSettings::rainWidth() const
{
    return ui->spinWidth->value();
}

int LayerParticleSettings::rainMovement() const
{
    return ui->spinMovement->value();
}

void LayerParticleSettings::setParticleCount(int count)
{
    ui->spinCount->setValue(count);
}

void LayerParticleSettings::setRainSpeed(int speed)
{
    ui->spinSpeed->setValue(speed);
}

void LayerParticleSettings::setRainDirection(int direction)
{
    ui->spinDirection->setValue(direction);
}

void LayerParticleSettings::setRainAngle(int angle)
{
    ui->spinAngle->setValue(angle);
}

void LayerParticleSettings::setRainColor(const QColor& color)
{
    ui->btnRainColor->setColor(color);
}

void LayerParticleSettings::setRainLength(int length)
{
    ui->spinLength->setValue(length);
}

void LayerParticleSettings::setRainOpacity(int opacity)
{
    ui->spinOpacity->setValue(opacity);
}

void LayerParticleSettings::setRainWidth(int width)
{
    ui->spinWidth->setValue(width);
}

void LayerParticleSettings::setRainMovement(int movement)
{
    ui->spinMovement->setValue(movement);
}

void LayerParticleSettings::applyPreset(int index)
{
    if(index == 1) // Rain
    {
        setParticleCount(1500);
        setRainSpeed(250);
        setRainDirection(0);
        setRainAngle(90);
        setRainLength(10);
        setRainOpacity(75);
        setRainWidth(1);
        setRainMovement(0);
        setRainColor(QColor(200, 200, 255, 180));
    }
    else if(index == 2) // Snow
    {
        setParticleCount(5000);
        setRainSpeed(30);
        setRainDirection(0);
        setRainAngle(90);
        setRainLength(1);
        setRainOpacity(100);
        setRainWidth(3);
        setRainMovement(2);
        setRainColor(QColor(255, 255, 255, 220));
    }

    // Reset combo to "Custom" so it doesn't stay on a preset name
    // while the user tweaks individual values
    ui->comboPreset->setCurrentIndex(0);
}
