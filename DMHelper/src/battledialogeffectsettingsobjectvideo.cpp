#include "battledialogeffectsettingsobjectvideo.h"
#include "ui_battledialogeffectsettingsobjectvideo.h"
#include "battledialogmodeleffectobjectvideo.h"
#include <QIntValidator>

BattleDialogEffectSettingsObjectVideo::BattleDialogEffectSettingsObjectVideo(const BattleDialogModelEffectObjectVideo& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettingsObjectVideo),
    _effect(effect)
{
    ui->setupUi(this);

    ui->edtName->setText(effect.getTip());
    ui->chkActive->setChecked(effect.getEffectActive());
    ui->chkVisible->setChecked(effect.getEffectVisible());
    ui->edtHeight->setValidator(new QIntValidator(1, 999, this));
    ui->edtHeight->setText(QString::number(effect.getSize()));
    ui->edtHeight->selectAll();
    ui->edtWidth->setValidator(new QIntValidator(1, 999, this));
    ui->edtWidth->setText(QString::number(effect.getWidth()));
    ui->edtWidth->selectAll();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(effect.getRotation()));
    ui->sliderTransparency->setSliderPosition(effect.getColor().alpha());
}

BattleDialogEffectSettingsObjectVideo::~BattleDialogEffectSettingsObjectVideo()
{
    delete ui;
}

void BattleDialogEffectSettingsObjectVideo::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectObjectVideo* other = dynamic_cast<BattleDialogModelEffectObjectVideo*>(&effect);
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

    if((!ui->edtHeight->text().isEmpty()) && (QString::number(effect.getSize()) != ui->edtHeight->text()))
        ui->edtHeight->setText(QString());

    if((!ui->edtWidth->text().isEmpty()) && (QString::number(effect.getWidth()) != ui->edtWidth->text()))
        ui->edtWidth->setText(QString());

    if((!ui->edtRotation->text().isEmpty()) && (QString::number(effect.getRotation()) != ui->edtRotation->text()))
        ui->edtRotation->setText(QString());

    if((!ui->chkColorize->isTristate()) && (other->getEffectVisible() != ui->chkVisible->isChecked()))
    {
        ui->chkColorize->setTristate();
        ui->chkColorize->setCheckState(Qt::PartiallyChecked);
    }
}

void BattleDialogEffectSettingsObjectVideo::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectObjectVideo* other = dynamic_cast<BattleDialogModelEffectObjectVideo*>(&effect);
    if(!other)
        return;

    if(ui->chkActive->checkState() != Qt::PartiallyChecked)
        effect.setEffectActive(isEffectActive());

    if(ui->chkVisible->checkState() != Qt::PartiallyChecked)
        effect.setEffectVisible(isEffectVisible());

    if(!ui->edtRotation->text().isEmpty())
        effect.setRotation(getRotation());

    if(!ui->edtHeight->text().isEmpty())
        effect.setSize(getHeightValue());

    if(!ui->edtWidth->text().isEmpty())
        effect.setWidth(getWidthValue());

    if(!ui->edtName->text().isEmpty())
        effect.setTip(getTip());
}

bool BattleDialogEffectSettingsObjectVideo::isEffectActive() const
{
    return ui->chkActive->isChecked();
}

bool BattleDialogEffectSettingsObjectVideo::isEffectVisible() const
{
    return ui->chkVisible->isChecked();
}

QString BattleDialogEffectSettingsObjectVideo::getTip() const
{
    return ui->edtName->text();
}

int BattleDialogEffectSettingsObjectVideo::getHeightValue() const
{
    return ui->edtHeight->text().toInt();
}

int BattleDialogEffectSettingsObjectVideo::getWidthValue() const
{
    return ui->edtWidth->text().toInt();
}

qreal BattleDialogEffectSettingsObjectVideo::getRotation() const
{
    return ui->edtRotation->text().toDouble();
}

DMHelper::TransparentType BattleDialogEffectSettingsObjectVideo::getEffectTransparencyType() const
{
    if(ui->btnRed->isChecked())
        return DMHelper::TransparentType_RedChannel;
    else if(ui->btnGreen->isChecked())
        return DMHelper::TransparentType_GreenChannel;
    else if(ui->btnBlue->isChecked())
        return DMHelper::TransparentType_BlueChannel;
    else if(ui->btnTransparent->isChecked())
        return DMHelper::TransparentType_TransparentColor;
    else
        return DMHelper::TransparentType_None;
}

QColor BattleDialogEffectSettingsObjectVideo::getTransparentColor() const
{
    return ui->btnTransparentColor->getColor();
}

qreal BattleDialogEffectSettingsObjectVideo::getTransparentTolerance() const
{
    return static_cast<qreal>(ui->slideTolerance->value()) / 100.0;
}

bool BattleDialogEffectSettingsObjectVideo::isColorize() const
{
    return ui->chkColorize->isChecked();
}

QColor BattleDialogEffectSettingsObjectVideo::getColorizeColor() const
{
    return ui->btnColorizeColor->getColor();
}
