#include "battledialogeffectsettings.h"
#include "ui_battledialogeffectsettings.h"
#include "battledialogmodeleffect.h"
#include <QColorDialog>
#include <QIntValidator>

BattleDialogEffectSettings::BattleDialogEffectSettings(int sizeval, qreal rotation, const QColor& color, QString tip, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BattleDialogEffectSettings),
    _color(color)
{
    ui->setupUi(this);

    ui->edtName->setText(tip);
    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    ui->edtSize->setText(QString::number(sizeval));
    ui->edtSize->selectAll();
    ui->edtWidth->setValidator(new QIntValidator(1,999,this));
    ui->edtWidth->setText(QString::number(5));
    ui->edtWidth->selectAll();
    ui->edtWidth->hide();
    ui->lblWidth->hide();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(rotation));
    ui->sliderTransparency->setSliderPosition(color.alpha());

    connect(ui->btnColor,SIGNAL(clicked()),this,SLOT(selectNewColor()));

    _color.setAlpha(255);
    setButtonColor(_color);
}

BattleDialogEffectSettings::BattleDialogEffectSettings(const BattleDialogModelEffect& effect, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BattleDialogEffectSettings),
    _color(effect.getColor())
{
    ui->setupUi(this);

    ui->edtName->setText(effect.getTip());
    ui->chkActive->setChecked(effect.getEffectActive());
    ui->chkVisible->setChecked(effect.getEffectVisible());
    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    ui->edtSize->setText(QString::number(effect.getSize()));
    ui->edtSize->selectAll();
    ui->edtWidth->setValidator(new QIntValidator(1,999,this));
    ui->edtWidth->setText(QString::number(effect.getWidth()));
    ui->edtWidth->selectAll();
    ui->edtWidth->hide();
    ui->lblWidth->hide();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(effect.getRotation()));
    ui->sliderTransparency->setSliderPosition(effect.getColor().alpha());

    connect(ui->btnColor,SIGNAL(clicked()),this,SLOT(selectNewColor()));

    _color.setAlpha(255);
    setButtonColor(_color);
}

BattleDialogEffectSettings::~BattleDialogEffectSettings()
{
    delete ui;
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

void BattleDialogEffectSettings::copyValues(BattleDialogModelEffect& effect)
{
    effect.setEffectActive(isEffectActive());
    effect.setEffectVisible(isEffectVisible());
    effect.setRotation(getRotation());
    effect.setSize(getSizeValue());
    effect.setWidth(getWidthValue());
    effect.setTip(getTip());

    QColor effectColor = getColor();
    effectColor.setAlpha(getAlpha());
    effect.setColor(effectColor);
}

void BattleDialogEffectSettings::setSizeLabel(const QString& sizeLabel)
{
    ui->lblSize->setText(sizeLabel);
}

void BattleDialogEffectSettings::setShowActive(bool show)
{
    ui->chkActive->setVisible(show);
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
        setButtonColor(newColor);
    }
}

void BattleDialogEffectSettings::setButtonColor(const QColor& color)
{
    _color = color;
    QString style = "background-image: url(); background-color: rgb(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + ");";
    ui->btnColor->setStyleSheet(style);
}
