#include "battledialogeffectsettings.h"
#include "ui_battledialogeffectsettings.h"
#include "battledialogmodeleffect.h"
#include <QColorDialog>
#include <QIntValidator>

BattleDialogEffectSettings::BattleDialogEffectSettings(int sizeval, qreal rotation, QColor color, QString tip, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BattleDialogEffectSettings),
    _color(color)
{
    ui->setupUi(this);

    ui->edtName->setText(tip);
    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    ui->edtSize->setText(QString::number(sizeval));
    ui->edtSize->selectAll();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(rotation));

    connect(ui->btnColor,SIGNAL(clicked()),this,SLOT(selectNewColor()));

    setButtonColor(_color);
}

BattleDialogEffectSettings::BattleDialogEffectSettings(const BattleDialogModelEffect& effect, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BattleDialogEffectSettings),
    _color(effect.getColor())
{
    ui->setupUi(this);

    ui->edtName->setText(effect.getTip());
    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    ui->edtSize->setText(QString::number(effect.getSize()));
    ui->edtSize->selectAll();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(effect.getRotation()));

    connect(ui->btnColor,SIGNAL(clicked()),this,SLOT(selectNewColor()));

    setButtonColor(_color);
}

BattleDialogEffectSettings::~BattleDialogEffectSettings()
{
    delete ui;
}

QString BattleDialogEffectSettings::getTip() const
{
    return ui->edtName->text();
}

int BattleDialogEffectSettings::getSizeValue() const
{
    return ui->edtSize->text().toInt();
}

qreal BattleDialogEffectSettings::getRotation() const
{
    return ui->edtRotation->text().toDouble();
}

QColor BattleDialogEffectSettings::getColor() const
{
    return _color;
}

void BattleDialogEffectSettings::copyValues(BattleDialogModelEffect& effect)
{
    effect.setRotation(getRotation());
    effect.setSize(getSizeValue());
    effect.setColor(getColor());
    effect.setTip(getTip());
}

void BattleDialogEffectSettings::selectNewColor()
{
    QColor newColor = QColorDialog::getColor(_color, this, QString("Select an effect color"));
    if(newColor.isValid())
    {
        setButtonColor(newColor);
    }
}

void BattleDialogEffectSettings::setButtonColor(QColor color)
{
    _color = color;
    QString style = "background-image: url(); background-color: rgb(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + ");";
    ui->btnColor->setStyleSheet(style);
}
