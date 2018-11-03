#include "widgetcharacter.h"
#include "ui_widgetcharacter.h"
#include "widgetcharacterinternal.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QIntValidator>

WidgetCharacter::WidgetCharacter(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::WidgetCharacter),
    _internals(0)
{
    ui->setupUi(this);

    connect(ui->edtInit,SIGNAL(editingFinished()),this,SLOT(edtInitiativeChanged(QString)));
    connect(ui->edtHP,SIGNAL(editingFinished()),this,SLOT(edtHPChanged(QString)));

    QValidator* valInit = new QIntValidator(-99, 99, this);
    ui->edtInit->setValidator(valInit);
    QValidator* valHitPoints = new QIntValidator(-10, 9999, this);
    ui->edtHP->setValidator(valHitPoints);
}

WidgetCharacter::~WidgetCharacter()
{
    delete _internals;
    delete ui;
}

BattleDialogModelCombatant* WidgetCharacter::getCombatant()
{
    if(_internals)
        return _internals->getCombatant();
    else
        return 0;
}

void WidgetCharacter::setInternals(WidgetCharacterInternal* internals)
{
    if(!internals)
        return;

    if(_internals)
        delete _internals;

    _internals = internals;
}

void WidgetCharacter::updateData()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    //updatePairData(pairName, _character->getName());
    //updatePairData(pairArmorClass, QString::number(_character->getArmorClass()));

    ui->edtName->setText(_internals->getCombatant()->getName());
    ui->edtHP->setText(QString::number(_internals->getCombatant()->getHitPoints()));
    ui->edtInit->setText(QString::number(_internals->getCombatant()->getInitiative()));

    loadImage();

    update();
}

void WidgetCharacter::leaveEvent(QEvent * event)
{
    _internals->leaveEvent(event);
}

void WidgetCharacter::mousePressEvent(QMouseEvent * event)
{
    _internals->mousePressEvent(event);
}

void WidgetCharacter::mouseReleaseEvent(QMouseEvent * event)
{
    _internals->mouseReleaseEvent(event);
}

void WidgetCharacter::mouseDoubleClickEvent(QMouseEvent *event)
{
    _internals->mouseDoubleClickEvent(event);
}

void WidgetCharacter::edtInitiativeChanged(const QString &text)
{
    if(_internals)
        _internals->initiativeChanged(text.toInt());
}

void WidgetCharacter::edtHPChanged(const QString &text)
{
    if(_internals)
        _internals->handleHitPointsChanged(text.toInt());
}

void WidgetCharacter::loadImage()
{
    if((_internals) && (_internals->getCombatant()))
    {
        ui->lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        ui->lblIcon->setPixmap(_internals->getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
    }

    /*
    if((_lblIcon)&&(getCombatant()))
    {
        _lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        _lblIcon->setPixmap(getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
    }
    */
}

// TODO: check highlighting
/*
    setWidgetHighlighted(_lblIcon, highlighted);
    setWidgetHighlighted(_lblInitName, highlighted);
    setWidgetHighlighted(_edtInit, highlighted);

    setPairHighlighted(pairName, highlighted);
    setPairHighlighted(pairArmorClass, highlighted);
*/
