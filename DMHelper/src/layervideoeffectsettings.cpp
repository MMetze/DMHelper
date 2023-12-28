#include "layervideoeffectsettings.h"
#include "ui_layervideoeffectsettings.h"
#include "colorpushbutton.h"

LayerVideoEffectSettings::LayerVideoEffectSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerVideoEffectSettings)
{
    ui->setupUi(this);

    ui->btnTransparentColor->setRotationVisible(false);
    ui->btnColorizeColor->setRotationVisible(false);

    ui->btnRed->setChecked(true);
    ui->btnColorizeColor->setChecked(false);

    connect(ui->btnTransparent, &QAbstractButton::toggled, ui->btnTransparentColor, &QAbstractButton::setEnabled);
    connect(ui->btnTransparent, &QAbstractButton::toggled, ui->slideTolerance, &QSlider::setEnabled);
}

LayerVideoEffectSettings::~LayerVideoEffectSettings()
{
    delete ui;
}

LayerVideoEffect::LayerVideoEffectType LayerVideoEffectSettings::getEffectType() const
{
    if(ui->btnRed->isChecked())
        return LayerVideoEffect::LayerVideoEffectType_Red;
    else if(ui->btnGreen->isChecked())
        return LayerVideoEffect::LayerVideoEffectType_Green;
    else if(ui->btnBlue->isChecked())
        return LayerVideoEffect::LayerVideoEffectType_Blue;
    else if(ui->btnTransparent->isChecked())
        return LayerVideoEffect::LayerVideoEffectType_TransparentColor;
    else
        return LayerVideoEffect::LayerVideoEffectType_None;
}

QColor LayerVideoEffectSettings::getTransparentColor() const
{
    return ui->btnTransparentColor->getColor();
}

qreal LayerVideoEffectSettings::getTransparentTolerance() const
{
    return static_cast<qreal>(ui->slideTolerance->value()) / 100.0;
}

bool LayerVideoEffectSettings::isColorize() const
{
    return ui->chkColorize->isChecked();
}

QColor LayerVideoEffectSettings::getColorizeColor() const
{
    return ui->btnColorizeColor->getColor();
}

void LayerVideoEffectSettings::setEffectType(LayerVideoEffect::LayerVideoEffectType effectType)
{
    switch(effectType)
    {
        case LayerVideoEffect::LayerVideoEffectType_Red:
            ui->btnRed->setChecked(true);
            break;
        case LayerVideoEffect::LayerVideoEffectType_Green:
            ui->btnGreen->setChecked(true);
            break;
        case LayerVideoEffect::LayerVideoEffectType_Blue:
            ui->btnBlue->setChecked(true);
            break;
        case LayerVideoEffect::LayerVideoEffectType_TransparentColor:
            ui->btnTransparent->setChecked(true);
            break;
        default:
            break;
    }
}

void LayerVideoEffectSettings::setTransparentColor(const QColor& transparentColor)
{
    ui->btnTransparentColor->setColor(transparentColor);
}

void LayerVideoEffectSettings::setTransparentTolerance(qreal transparentTolerance)
{
    ui->slideTolerance->setValue(static_cast<int>(transparentTolerance * 100.0));
}

void LayerVideoEffectSettings::setColorize(bool colorize)
{
    ui->chkColorize->setChecked(colorize);
}

void LayerVideoEffectSettings::setColorizeColor(const QColor& colorizeColor)
{
    ui->btnColorizeColor->setColor(colorizeColor);
}
