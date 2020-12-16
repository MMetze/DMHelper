#include "widgetcharacter.h"
#include "ui_widgetcharacter.h"
#include "widgetcharacterinternal.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "character.h"
#include <QIntValidator>
#include <QDebug>

WidgetCharacter::WidgetCharacter(QWidget *parent) :
    CombatantWidget(parent),
    ui(new Ui::WidgetCharacter),
    _internals(nullptr)
{
    ui->setupUi(this);

    connect(ui->edtInit,SIGNAL(editingFinished()),this,SLOT(edtInitiativeChanged()));
    connect(ui->edtHP,SIGNAL(editingFinished()),this,SLOT(edtHPChanged()));

    connect(ui->chkKnown, SIGNAL(clicked(bool)), this, SIGNAL(isKnownChanged(bool)));
    connect(ui->chkVisible, SIGNAL(clicked(bool)), this, SIGNAL(isShownChanged(bool)));

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
        return nullptr;
}

void WidgetCharacter::setInternals(WidgetCharacterInternal* internals)
{
    if(!internals)
        return;

    if(_internals)
        delete _internals;

    _internals = internals;

    BattleDialogModelCharacter* characterCombatant = dynamic_cast<BattleDialogModelCharacter*>(_internals->getCombatant());
    if(characterCombatant)
    {
        connect(ui->chkKnown,SIGNAL(clicked(bool)),characterCombatant,SLOT(setKnown(bool)));
        connect(ui->chkVisible,SIGNAL(clicked(bool)),characterCombatant,SLOT(setShown(bool)));

        if(characterCombatant->getCombatant())
            connect(characterCombatant->getCombatant(),SIGNAL(dirty()),this,SLOT(updateData()));
        else
            qDebug() << "[Character Widget] a valid combatant could not be found!";
    }

    readInternals();
}

bool WidgetCharacter::isShown()
{
    return ui->chkVisible->isChecked();
}

bool WidgetCharacter::isKnown()
{
    return ui->chkKnown->isChecked();
}

void WidgetCharacter::updateData()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    readInternals();
    update();
}

void WidgetCharacter::leaveEvent(QEvent * event)
{
    if(_internals)
        _internals->leaveEvent(event);

    CombatantWidget::leaveEvent(event);
}

void WidgetCharacter::mousePressEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mousePressEvent(event);

    CombatantWidget::mousePressEvent(event);
}

void WidgetCharacter::mouseReleaseEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mouseReleaseEvent(event);

    CombatantWidget::mouseReleaseEvent(event);
}

void WidgetCharacter::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(_internals)
        _internals->mouseDoubleClickEvent(event);

    CombatantWidget::mouseDoubleClickEvent(event);
}

void WidgetCharacter::edtInitiativeChanged()
{
    if(_internals)
        _internals->initiativeChanged(ui->edtInit->text().toInt());
}

void WidgetCharacter::edtHPChanged()
{
    if(_internals)
        _internals->handleHitPointsChanged(ui->edtHP->text().toInt());
}

void WidgetCharacter::readInternals()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    loadImage();

    ui->edtName->setText(_internals->getCombatant()->getName());
    ui->edtName->home(false);
    ui->edtAC->setText(QString::number(_internals->getCombatant()->getArmorClass()));
    ui->edtHP->setText(QString::number(_internals->getCombatant()->getHitPoints()));
    ui->edtInit->setText(QString::number(_internals->getCombatant()->getInitiative()));
    ui->chkKnown->setChecked(_internals->getCombatant()->getKnown());
    ui->chkVisible->setChecked(_internals->getCombatant()->getShown());
}

void WidgetCharacter::loadImage()
{
    if((_internals) && (_internals->getCombatant()))
    {
        ui->lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        ui->lblIcon->setPixmap(_internals->getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
    }
}
