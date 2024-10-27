#include "battledialogeffectsettings.h"
#include "ui_battledialogeffectsettings.h"
#include "battledialogmodeleffect.h"
#include <QColorDialog>
#include <QIntValidator>

BattleDialogEffectSettings::BattleDialogEffectSettings(const BattleDialogModelEffect& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettings),
    _color(effect.getColor())
{
    ui->setupUi(this);

    ui->edtName->setText(effect.getTip());
    ui->chkActive->setChecked(effect.getEffectActive());
    ui->chkVisible->setChecked(effect.getEffectVisible());
    ui->edtSize->setValidator(new QIntValidator(1, 999, this));
    ui->edtSize->setText(QString::number(effect.getSize()));
    ui->edtSize->selectAll();
    ui->edtWidth->setValidator(new QIntValidator(1, 999, this));
    ui->edtWidth->setText(QString::number(effect.getWidth()));
    ui->edtWidth->selectAll();
    ui->edtWidth->hide();
    ui->lblWidth->hide();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(effect.getRotation()));
    ui->sliderTransparency->setSliderPosition(effect.getColor().alpha());

    connect(ui->btnColor, SIGNAL(clicked()), this, SLOT(selectNewColor()));

    _color.setAlpha(255);
    setButtonColor(_color);
}

BattleDialogEffectSettings::~BattleDialogEffectSettings()
{
    delete ui;
}

void BattleDialogEffectSettings::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
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

    if((!ui->edtWidth->text().isEmpty()) && (QString::number(effect.getWidth()) != ui->edtWidth->text()))
        ui->edtWidth->setText(QString());

    if((!ui->edtRotation->text().isEmpty()) && (QString::number(effect.getRotation()) != ui->edtRotation->text()))
        ui->edtRotation->setText(QString());

    if((_color.alpha() != 0) && ((_color.red() != effect.getColor().red()) || (_color.green() != effect.getColor().green()) || (_color.blue() != effect.getColor().blue())))
    {
        _color.setAlpha(0);
        setButtonColor(_color);
    }
}

void BattleDialogEffectSettings::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    if(ui->chkActive->checkState() != Qt::PartiallyChecked)
        effect.setEffectActive(isEffectActive());

    if(ui->chkVisible->checkState() != Qt::PartiallyChecked)
        effect.setEffectVisible(isEffectVisible());

    if(!ui->edtRotation->text().isEmpty())
        effect.setRotation(getRotation());

    if(!ui->edtSize->text().isEmpty())
        effect.setSize(getSizeValue());

    if(!ui->edtWidth->text().isEmpty())
        effect.setWidth(getWidthValue());

    if(!ui->edtName->text().isEmpty())
        effect.setTip(getTip());

    QColor effectColor = (_color.alpha() == 255) ? getColor() : effect.getColor();
    effectColor.setAlpha(getAlpha());
    effect.setColor(effectColor);
}

bool BattleDialogEffectSettings::isEffectActive() const
{
    return ui->chkActive->isChecked();
}

bool BattleDialogEffectSettings::isEffectVisible() const
{
    return ui->chkVisible->isChecked();
}

QString BattleDialogEffectSettings::getTip() const
{
    return ui->edtName->text();
}

int BattleDialogEffectSettings::getSizeValue() const
{
    return ui->edtSize->text().toInt();
}

int BattleDialogEffectSettings::getWidthValue() const
{
    return ui->edtWidth->text().toInt();
}

qreal BattleDialogEffectSettings::getRotation() const
{
    return ui->edtRotation->text().toDouble();
}

QColor BattleDialogEffectSettings::getColor() const
{
    return _color;
}

int BattleDialogEffectSettings::getAlpha() const
{
    return ui->sliderTransparency->sliderPosition();
}

void BattleDialogEffectSettings::setSizeLabel(const QString& sizeLabel)
{
    ui->lblSize->setText(sizeLabel);
}

void BattleDialogEffectSettings::setShowActive(bool show)
{
    ui->chkActive->setVisible(show);
    ui->lblActive->setVisible(show);
}

void BattleDialogEffectSettings::setShowWidth(bool show)
{
    ui->edtWidth->setVisible(show);
    ui->lblWidth->setVisible(show);
}

void BattleDialogEffectSettings::setShowColor(bool show)
{
    ui->btnColor->setVisible(show);
    ui->lblColor->setVisible(show);
}

void BattleDialogEffectSettings::selectNewColor()
{
    QColor newColor = QColorDialog::getColor(_color, this, QString("Select an effect color"));
    if(newColor.isValid())
    {
        newColor.setAlpha(255);
        setButtonColor(newColor);
    }
}

void BattleDialogEffectSettings::setButtonColor(const QColor& color)
{
    _color = color;
    QString style;
    if(_color.alpha() == 0)
        style = "background-image: url(); background-color: rgb(242,242,242);";
    else
        style = "background-image: url(); background-color: rgb(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + ");";
    ui->btnColor->setStyleSheet(style);
}
