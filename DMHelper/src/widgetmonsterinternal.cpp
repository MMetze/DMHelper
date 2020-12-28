#include "widgetmonsterinternal.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "widgetmonster.h"
#include "monsterclass.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QDebug>

WidgetMonsterInternal::WidgetMonsterInternal(BattleDialogModelMonsterBase* monster, WidgetMonster* parent) :
    WidgetCombatantBase(parent),
    _widgetParent(parent),
    _monster(monster),
    _legendaryMaximum(3)
{
    if(_widgetParent)
        _widgetParent->setInternals(this);

    if(_monster)
    {
        connect(_monster, &BattleDialogModelMonsterBase::dataChanged, this, &WidgetMonsterInternal::updateData);
        connect(_monster, &BattleDialogModelCombatant::initiativeChanged, this, &WidgetMonsterInternal::updateData);
    }
}

BattleDialogModelCombatant* WidgetMonsterInternal::getCombatant()
{
    return _monster;
}

QFrame* WidgetMonsterInternal::getFrame()
{
    return _widgetParent;
}

int WidgetMonsterInternal::getInitiative() const
{
    if(_monster)
        return _monster->getInitiative();
    else
        return 0;
}

bool WidgetMonsterInternal::isShown()
{
    if(_monster)
        return _monster->getShown();
    else
        return true;
}

bool WidgetMonsterInternal::isKnown()
{
    if(_monster)
        return _monster->getKnown();
    else
        return true;
}

void WidgetMonsterInternal::setLegendaryMaximum(int legendaryMaximum)
{
    _legendaryMaximum = legendaryMaximum;
    resetLegendary();
}

int WidgetMonsterInternal::getLegendaryMaximum() const
{
    return _legendaryMaximum;
}

void WidgetMonsterInternal::updateData()
{
    if(_widgetParent)
        _widgetParent->updateData();
}

void WidgetMonsterInternal::setInitiative(int initiative)
{
    Q_UNUSED(initiative);

    if(!_widgetParent || !_monster)
        return;

    if(initiative != _monster->getInitiative())
        _monster->setInitiative(initiative);
}

void WidgetMonsterInternal::setHitPoints(int hp)
{
    Q_UNUSED(hp);

    if(!_widgetParent || !_monster)
        return;

    if(hp != _monster->getHitPoints())
        _monster->setHitPoints(hp);
}

void WidgetMonsterInternal::decrementLegendary()
{
    if(!_monster)
        return;

    if(_monster->getLegendaryCount() <= 0)
        resetLegendary();
    else
        _monster->setLegendaryCount(_monster->getLegendaryCount() - 1);
}

void WidgetMonsterInternal::resetLegendary()
{
    _monster->setLegendaryCount(_legendaryMaximum);
}

void WidgetMonsterInternal::executeDoubleClick()
{
    if((_monster) && (_monster->getMonsterClass()))
        emit clicked(_monster->getMonsterClass()->getName());
    else
        qDebug() << "[Widget Monster Internal] no valid monster class found!";
}

