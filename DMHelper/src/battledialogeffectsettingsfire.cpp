#include "battledialogeffectsettingsfire.h"
#include "ui_battledialogeffectsettingsfire.h"
#include "battledialogmodeleffectfire.h"
#include <QIntValidator>
#include <QDoubleValidator>

BattleDialogEffectSettingsFire::BattleDialogEffectSettingsFire(const BattleDialogModelEffectFire& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettingsFire)
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

    ui->spinIntensity->setValue(effect.getIntensity());
    ui->spinFlickerSpeed->setValue(effect.getFlickerSpeed());

    ui->btnDarkColor->setRotationVisible(false);
    QColor darkColor = effect.getDarkColor();
    darkColor.setAlpha(255);
    ui->btnDarkColor->setColor(darkColor);

    ui->btnLightColor->setRotationVisible(false);
    QColor lightColor = effect.getLightColor();
    lightColor.setAlpha(255);
    ui->btnLightColor->setColor(lightColor);
}

BattleDialogEffectSettingsFire::~BattleDialogEffectSettingsFire()
{
    delete ui;
}

void BattleDialogEffectSettingsFire::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectFire* other = dynamic_cast<BattleDialogModelEffectFire*>(&effect);
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

void BattleDialogEffectSettingsFire::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectFire* fireEffect = dynamic_cast<BattleDialogModelEffectFire*>(&effect);
    if(!fireEffect)
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

    QColor effectColor = effect.getColor();
    effectColor.setAlpha(ui->sliderOpacity->sliderPosition());
    effect.setColor(effectColor);

    fireEffect->setIntensity(ui->spinIntensity->value());
    fireEffect->setFlickerSpeed(ui->spinFlickerSpeed->value());

    QColor dc = ui->btnDarkColor->getColor();
    dc.setAlpha(fireEffect->getDarkColor().alpha());
    fireEffect->setDarkColor(dc);

    QColor lc = ui->btnLightColor->getColor();
    lc.setAlpha(fireEffect->getLightColor().alpha());
    fireEffect->setLightColor(lc);
}
