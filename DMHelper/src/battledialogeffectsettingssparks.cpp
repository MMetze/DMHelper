#include "battledialogeffectsettingssparks.h"
#include "ui_battledialogeffectsettingssparks.h"
#include "battledialogmodeleffectsparks.h"
#include <QIntValidator>
#include <QDoubleValidator>

BattleDialogEffectSettingsSparks::BattleDialogEffectSettingsSparks(const BattleDialogModelEffectSparks& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettingsSparks)
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
    QColor sparkColor = effect.getColor();
    sparkColor.setAlpha(255);
    ui->btnColor->setColor(sparkColor);

    ui->spinParticleCount->setValue(effect.getParticleCount());
    ui->spinGlowRadius->setValue(effect.getGlowRadius());
    ui->spinGlowOpacity->setValue(effect.getGlowOpacity());
    ui->chkArcFalloff->setChecked(effect.getArcFalloff());
    ui->chkFadeDistance->setChecked(effect.getFadeDistance());
    ui->spinSparkSpeed->setValue(effect.getSparkSpeed());
    ui->spinWindDirection->setValue(effect.getWindDirection());
    ui->spinWindStrength->setValue(effect.getWindStrength());
}

BattleDialogEffectSettingsSparks::~BattleDialogEffectSettingsSparks()
{
    delete ui;
}

void BattleDialogEffectSettingsSparks::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectSparks* other = dynamic_cast<BattleDialogModelEffectSparks*>(&effect);
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

void BattleDialogEffectSettingsSparks::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectSparks* sparksEffect = dynamic_cast<BattleDialogModelEffectSparks*>(&effect);
    if(!sparksEffect)
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

    sparksEffect->setParticleCount(ui->spinParticleCount->value());
    sparksEffect->setGlowRadius(ui->spinGlowRadius->value());
    sparksEffect->setGlowOpacity(ui->spinGlowOpacity->value());
    sparksEffect->setArcFalloff(ui->chkArcFalloff->isChecked());
    sparksEffect->setFadeDistance(ui->chkFadeDistance->isChecked());
    sparksEffect->setSparkSpeed(ui->spinSparkSpeed->value());
    sparksEffect->setWindDirection(ui->spinWindDirection->value());
    sparksEffect->setWindStrength(ui->spinWindStrength->value());
}
