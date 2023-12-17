#include "layervideoeffectsettings.h"
#include "ui_layervideoeffectsettings.h"

LayerVideoEffectSettings::LayerVideoEffectSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerVideoEffectSettings)
{
    ui->setupUi(this);

    ui->btnTransparentColor->setRotationVisible(false);
    ui->btnColorize->setRotationVisible(false);

    ui->btnRed->setChecked(true);
    ui->btnColorize->setChecked(false);
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
    else if(ui->btnTransparentColor->isChecked())
        return LayerVideoEffect::LayerVideoEffectType_TransparentColor;
    else
        return LayerVideoEffect::LayerVideoEffectType_None;
}

bool LayerVideoEffectSettings::isColorize() const
{
    return ui->btnColorize->isChecked();
}

QColor LayerVideoEffectSettings::getTransparentColor() const
{
    return ui->btnTransparentColor->getColor();
}

QColor LayerVideoEffectSettings::getColorizeColor() const
{
    return ui->btnColorize->getColor();
}
