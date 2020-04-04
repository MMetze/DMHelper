#include "battledialogeventdamage.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

BattleDialogEventDamage::BattleDialogEventDamage(QUuid combatant, QUuid target, int damage) :
    BattleDialogEvent(),
    _combatant(combatant),
    _target(target),
    _damage(damage)
{
}

BattleDialogEventDamage::BattleDialogEventDamage(const QDomElement& element) :
    BattleDialogEvent(element),
    _combatant(element.attribute("combatant")),
    _target(element.attribute("target")),
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

BattleDialogEvent* BattleDialogEventDamage::clone()
{
    return new BattleDialogEventDamage(*this);
}

QDomElement BattleDialogEventDamage::outputXML(QDomElement &element, bool isExport)
{
    Q_UNUSED(isExport);

    element.setAttribute( "combatant", _combatant.toString() );
    element.setAttribute( "target", _target.toString() );
    element.setAttribute( "damage", _damage );

    return element;
}

QUuid BattleDialogEventDamage::getCombatant() const
{
    return _combatant;
}

QUuid BattleDialogEventDamage::getTarget() const
{
    return _target;
}

int BattleDialogEventDamage::getDamage() const
{
    return _damage;
}
