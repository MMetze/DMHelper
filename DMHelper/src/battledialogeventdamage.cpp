#include "battledialogeventdamage.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

BattleDialogEventDamage::BattleDialogEventDamage(int combatant, int target, int damage) :
    BattleDialogEvent(),
    _combatant(combatant),
    _target(target),
    _damage(damage)
{
}

BattleDialogEventDamage::BattleDialogEventDamage(const QDomElement& element) :
    BattleDialogEvent(element),
    _combatant(element.attribute("combatant").toInt()),
    _target(element.attribute("target").toInt()),
    _damage(element.attribute("damage").toInt())
{
}

BattleDialogEventDamage::BattleDialogEventDamage(const BattleDialogEventDamage& other) :
    BattleDialogEvent(other),
    _combatant(other._combatant),
    _target(other._target),
    _damage(other._damage)
{
}

BattleDialogEventDamage::~BattleDialogEventDamage()
{
}

int BattleDialogEventDamage::getType() const
{
    return DMHelper::BattleEvent_Damage;
}

void BattleDialogEventDamage::outputXML(QDomElement &element)
{
    element.setAttribute( "combatant", _combatant );
    element.setAttribute( "target", _target );
    element.setAttribute( "damage", _damage );
}

int BattleDialogEventDamage::getCombatant() const
{
    return _combatant;
}

int BattleDialogEventDamage::getTarget() const
{
    return _target;
}

int BattleDialogEventDamage::getDamage() const
{
    return _damage;
}
