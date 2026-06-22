#include "battledialogeffectsettingssmoke.h"
#include "ui_battledialogeffectsettingssmoke.h"
#include "battledialogmodeleffectsmoke.h"
#include <QIntValidator>
#include <QDoubleValidator>

BattleDialogEffectSettingsSmoke::BattleDialogEffectSettingsSmoke(const BattleDialogModelEffectSmoke& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettingsSmoke)
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

    ui->spinDensity->setValue(effect.getDensity());
    ui->spinBillowFactor->setValue(effect.getBillowFactor());
    ui->spinWindDirection->setValue(effect.getWindDirection());
    ui->spinWindStrength->setValue(effect.getWindStrength());

    ui->btnCenterColor->setRotationVisible(false);
    QColor centerColor = effect.getCenterColor();
    centerColor.setAlpha(255);
    ui->btnCenterColor->setColor(centerColor);

    ui->btnEdgeColor->setRotationVisible(false);
    QColor edgeColor = effect.getEdgeColor();
    edgeColor.setAlpha(255);
    ui->btnEdgeColor->setColor(edgeColor);
}

BattleDialogEffectSettingsSmoke::~BattleDialogEffectSettingsSmoke()
{
    delete ui;
}

void BattleDialogEffectSettingsSmoke::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectSmoke* other = dynamic_cast<BattleDialogModelEffectSmoke*>(&effect);
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

void BattleDialogEffectSettingsSmoke::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectSmoke* smokeEffect = dynamic_cast<BattleDialogModelEffectSmoke*>(&effect);
    if(!smokeEffect)
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

    smokeEffect->setDensity(ui->spinDensity->value());
    smokeEffect->setBillowFactor(ui->spinBillowFactor->value());
    smokeEffect->setWindDirection(ui->spinWindDirection->value());
    smokeEffect->setWindStrength(ui->spinWindStrength->value());

    QColor cc = ui->btnCenterColor->getColor();
    cc.setAlpha(smokeEffect->getCenterColor().alpha());
    smokeEffect->setCenterColor(cc);

    QColor ec = ui->btnEdgeColor->getColor();
    ec.setAlpha(smokeEffect->getEdgeColor().alpha());
    smokeEffect->setEdgeColor(ec);
}
