#include "combatantwidgetinternalsmonster.h"
#include "battledialogmodelmonsterbase.h"
#include "combatantwidgetmonster.h"
#include "monsterclassv2.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QDebug>

CombatantWidgetInternalsMonster::CombatantWidgetInternalsMonster(BattleDialogModelMonsterBase* monster, CombatantWidgetMonster* parent) :
    CombatantWidgetInternals(parent),
    _widgetParent(parent),
    _monster(monster),
    _legendaryMaximum(3)
{
    if(_widgetParent)
        _widgetParent->setInternals(this);

    if(_monster)
    {
        connect(_monster, &BattleDialogModelMonsterBase::dataChanged, this, &CombatantWidgetInternalsMonster::updateData);
        connect(_monster, &BattleDialogModelMonsterBase::imageChanged, this, &CombatantWidgetInternalsMonster::updateImage);
        connect(_monster, &BattleDialogModelCombatant::initiativeChanged, this, &CombatantWidgetInternalsMonster::updateData);
    }
}

BattleDialogModelCombatant* CombatantWidgetInternalsMonster::getCombatant()
{
    return _monster;
}

QFrame* CombatantWidgetInternalsMonster::getFrame()
{
    return _widgetParent;
}

int CombatantWidgetInternalsMonster::getInitiative() const
{
    if(_monster)
        return _monster->getInitiative();
    else
        return 0;
}

bool CombatantWidgetInternalsMonster::isShown()
{
    if(_monster)
        return _monster->getShown();
    else
        return true;
}

bool CombatantWidgetInternalsMonster::isKnown()
{
    if(_monster)
        return _monster->getKnown();
    else
        return true;
}

void CombatantWidgetInternalsMonster::setLegendaryMaximum(int legendaryMaximum)
{
    _legendaryMaximum = legendaryMaximum;
    resetLegendary();
}

int CombatantWidgetInternalsMonster::getLegendaryMaximum() const
{
    return _legendaryMaximum;
}

void CombatantWidgetInternalsMonster::updateImage()
{
    if(_widgetParent)
    {
        _widgetParent->clearImage();
        _widgetParent->updateData();
    }
}

void CombatantWidgetInternalsMonster::updateData()
{
    if(_widgetParent)
        _widgetParent->updateData();
}

void CombatantWidgetInternalsMonster::setInitiative(int initiative)
{
    Q_UNUSED(initiative);

    if(!_widgetParent || !_monster)
        return;

    if(initiative != _monster->getInitiative())
        _monster->setInitiative(initiative);
}

void CombatantWidgetInternalsMonster::setHitPoints(int hp)
{
    if(!_widgetParent || !_monster)
        return;

    if(hp != _monster->getHitPoints())
        _monster->setHitPoints(hp);
}

void CombatantWidgetInternalsMonster::executeDoubleClick()
{
    if((_monster) && (_monster->getMonsterClass()))
        emit clicked(_monster->getMonsterClass()->getStringValue("name"));
    else
        qDebug() << "[Widget Monster Internal] no valid monster class found!";
}

void CombatantWidgetInternalsMonster::decrementLegendary()
{
    if(!_monster)
        return;

    if(_monster->getLegendaryCount() <= 0)
        resetLegendary();
    else
        _monster->setLegendaryCount(_monster->getLegendaryCount() - 1);
}

void CombatantWidgetInternalsMonster::resetLegendary()
{
    _monster->setLegendaryCount(_legendaryMaximum);
}
