#include "battledialogeffectsettingslight.h"
#include "ui_battledialogeffectsettingslight.h"
#include "battledialogmodeleffectlight.h"
#include <QIntValidator>
#include <QDoubleValidator>

BattleDialogEffectSettingsLight::BattleDialogEffectSettingsLight(const BattleDialogModelEffectLight& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettingsLight)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    ui->edtName->setText(effect.getTip());
    ui->chkActive->setChecked(effect.getEffectActive());
    ui->chkVisible->setChecked(effect.getEffectVisible());
    ui->edtSize->setValidator(new QIntValidator(1, 999, this));
    ui->edtSize->setText(QString::number(effect.getSize()));
    ui->edtSize->selectAll();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(effect.getRotation()));
    ui->sliderOpacity->setSliderPosition(effect.getColor().alpha());

    ui->btnColor->setRotationVisible(false);
    QColor lightColor = effect.getColor();
    lightColor.setAlpha(255);
    ui->btnColor->setColor(lightColor);

    ui->cmbGradientType->setCurrentIndex(effect.getGradientType());
    ui->chkFlickerEnabled->setChecked(effect.getFlickerEnabled());
    ui->spinFlickerFrequency->setValue(effect.getFlickerFrequency());
    ui->spinDimAmplitude->setValue(effect.getDimAmplitude());
}

BattleDialogEffectSettingsLight::~BattleDialogEffectSettingsLight()
{
    delete ui;
}

void BattleDialogEffectSettingsLight::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectLight* other = dynamic_cast<BattleDialogModelEffectLight*>(&effect);
    if(!other)
        return;

    if((!ui->edtName->text().isEmpty()) && (effect.getTip() != ui->edtName->text()))
        ui->edtName->setText(QString());

    if((!ui->chkActive->isTristate()) && (effect.getEffectActive() != ui->chkActive->isChecked()))
    {
        ui->chkActive->setTristate();
        ui->chkActive->setCheckState(Qt::PartiallyChecked);
    }

    if((!ui->chkVisible->isTristate()) && (effect.getEffectVisible() != ui->chkVisible->isChecked()))
    {
        ui->chkVisible->setTristate();
        ui->chkVisible->setCheckState(Qt::PartiallyChecked);
    }

    if((!ui->edtSize->text().isEmpty()) && (QString::number(effect.getSize()) != ui->edtSize->text()))
        ui->edtSize->setText(QString());

    if((!ui->edtRotation->text().isEmpty()) && (QString::number(effect.getRotation()) != ui->edtRotation->text()))
        ui->edtRotation->setText(QString());
}

void BattleDialogEffectSettingsLight::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectLight* lightEffect = dynamic_cast<BattleDialogModelEffectLight*>(&effect);
    if(!lightEffect)
        return;

    if(ui->chkActive->checkState() != Qt::PartiallyChecked)
        effect.setEffectActive(ui->chkActive->isChecked());

    if(ui->chkVisible->checkState() != Qt::PartiallyChecked)
        effect.setEffectVisible(ui->chkVisible->isChecked());

    if(!ui->edtRotation->text().isEmpty())
        effect.setRotation(ui->edtRotation->text().toDouble());

    if(!ui->edtSize->text().isEmpty())
        effect.setSize(ui->edtSize->text().toInt());

    if(!ui->edtName->text().isEmpty())
        effect.setTip(ui->edtName->text());

    QColor effectColor = ui->btnColor->getColor();
    effectColor.setAlpha(ui->sliderOpacity->sliderPosition());
    effect.setColor(effectColor);

    lightEffect->setGradientType(ui->cmbGradientType->currentIndex());
    lightEffect->setFlickerEnabled(ui->chkFlickerEnabled->isChecked());
    lightEffect->setFlickerFrequency(ui->spinFlickerFrequency->value());
    lightEffect->setDimAmplitude(ui->spinDimAmplitude->value());
}
