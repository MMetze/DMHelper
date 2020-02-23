#include "widgetmonster.h"
#include "ui_widgetmonster.h"
#include "widgetmonsterinternal.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclass.h"
#include <QIntValidator>
#include <QDebug>

WidgetMonster::WidgetMonster(QWidget *parent) :
    CombatantWidget(parent),
    ui(new Ui::WidgetMonster),
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

WidgetMonster::~WidgetMonster()
{
    delete _internals;
    delete ui;
}

BattleDialogModelCombatant* WidgetMonster::getCombatant()
{
    if(_internals)
        return _internals->getCombatant();
    else
        return nullptr;
}

void WidgetMonster::setInternals(WidgetMonsterInternal* internals)
{
    if((!internals) || !internals->getCombatant())
        return;

    if(_internals)
        delete _internals;

    _internals = internals;

    BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(_internals->getCombatant());
    if(monsterCombatant)
    {
        connect(ui->chkKnown,SIGNAL(clicked(bool)),monsterCombatant,SLOT(setKnown(bool)));
        connect(ui->chkVisible,SIGNAL(clicked(bool)),monsterCombatant,SLOT(setShown(bool)));

        if(monsterCombatant->getCombatant())
            connect(monsterCombatant->getCombatant(),SIGNAL(dirty()),this,SLOT(updateData()));
        else if (monsterCombatant->getMonsterClass())
            connect(monsterCombatant->getMonsterClass(),SIGNAL(dirty()),this,SLOT(updateData()));
        else
            qDebug() << "[Monster Widget] neither valid combatant nor monster class found!";

        if((monsterCombatant->getMonsterClass()) && (monsterCombatant->getMonsterClass()->getLegendary()))
        {
            _internals->resetLegendary();
            connect(ui->btnLegendary, SIGNAL(clicked(bool)), _internals, SLOT(decrementLegendary()));
            ui->btnLegendary->show();
        }
        else
        {
            ui->btnLegendary->hide();
        }
    }

    readInternals();
}

bool WidgetMonster::isShown()
{
    return ui->chkVisible->isChecked();
}

bool WidgetMonster::isKnown()
{
    return ui->chkKnown->isChecked();
}

void WidgetMonster::updateData()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    readInternals();
    update();
}

void WidgetMonster::setActive(bool active)
{
    if(_internals)
        _internals->resetLegendary();

    CombatantWidget::setActive(active);
}

void WidgetMonster::leaveEvent(QEvent * event)
{
    if(_internals)
        _internals->leaveEvent(event);
}

void WidgetMonster::mousePressEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mousePressEvent(event);
}

void WidgetMonster::mouseReleaseEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mouseReleaseEvent(event);
}

void WidgetMonster::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(_internals)
        _internals->mouseDoubleClickEvent(event);
}

void WidgetMonster::edtInitiativeChanged()
{
    if(_internals)
        _internals->initiativeChanged(ui->edtInit->text().toInt());
}

void WidgetMonster::edtHPChanged()
{
    if(_internals)
        _internals->handleHitPointsChanged(ui->edtHP->text().toInt());
}

void WidgetMonster::readInternals()
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

    BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(_internals->getCombatant());
    if((monsterCombatant) && (monsterCombatant->getMonsterClass()) && (monsterCombatant->getMonsterClass()->getLegendary()))
        ui->btnLegendary->setText(QString("L: ") + QString::number(monsterCombatant->getLegendaryCount()));
}

void WidgetMonster::loadImage()
{
    if(ui->lblIcon->pixmap())
        return;

    if((_internals) && (_internals->getCombatant()))
    {
        ui->lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        ui->lblIcon->setPixmap(_internals->getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
        emit imageChanged(_internals->getCombatant());
    }
}
