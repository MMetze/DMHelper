#include "battledialogmodelcombatant.h"
#include <QDomElement>

BattleDialogModelCombatant::BattleDialogModelCombatant(const QString& name, QObject *parent) :
    BattleDialogModelObject(QPointF(), name, parent),
    _combatant(nullptr),
    _initiative(0),
    _moved(0.0),
    _isShown(true),
    _isKnown(true),
    _isSelected(false)
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant) :
    BattleDialogModelObject(),
    _combatant(combatant),
    _initiative(0),
    _moved(0.0),
    _isShown(true),
    _isKnown(true),
    _isSelected(false)
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant, int initiative, const QPointF& position) :
    BattleDialogModelObject(position),
    _combatant(combatant),
    _initiative(initiative),
    _moved(0.0),
    _isShown(true),
    _isKnown(true),
    _isSelected(false)
{
}

BattleDialogModelCombatant::~BattleDialogModelCombatant()
{
}

void BattleDialogModelCombatant::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelObject::inputXML(element, isImport);

    _initiative = element.attribute("initiative", QString::number(0)).toInt();
    _isShown = static_cast<bool>(element.attribute("isShown", QString::number(1)).toInt());
    _isKnown = static_cast<bool>(element.attribute("isKnown", QString::number(1)).toInt());
}

void BattleDialogModelCombatant::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelCombatant* otherCombatant = dynamic_cast<const BattleDialogModelCombatant*>(other);
    if(!otherCombatant)
        return;

    _combatant = otherCombatant->_combatant;
    _initiative = otherCombatant->_initiative;
    _moved = otherCombatant->_moved;
    _isShown = otherCombatant->_isShown;
    _isKnown = otherCombatant->_isKnown;
    _isSelected = otherCombatant->_isSelected;

    BattleDialogModelObject::copyValues(other);
}

int BattleDialogModelCombatant::getObjectType() const
{
    return DMHelper::CampaignType_BattleContentCombatant;
}

bool BattleDialogModelCombatant::getShown() const
{
    return _isShown;
}

bool BattleDialogModelCombatant::getKnown() const
{
    return _isKnown;
}

bool BattleDialogModelCombatant::getSelected() const
{
    return _isSelected;
}

int BattleDialogModelCombatant::getInitiative() const
{
    return _initiative;
}

void BattleDialogModelCombatant::setInitiative(int initiative)
{
    if(_initiative != initiative)
    {
        _initiative = initiative;
        emit initiativeChanged(this);
    }
}

Combatant* BattleDialogModelCombatant::getCombatant() const
{
    return _combatant;
}

int BattleDialogModelCombatant::getAbilityValue(Combatant::Ability ability) const
{
    switch(ability)
    {
        case Combatant::Ability_Strength:
            return getStrength();
        case Combatant::Ability_Dexterity:
            return getDexterity();
        case Combatant::Ability_Constitution:
            return getConstitution();
        case Combatant::Ability_Intelligence:
            return getIntelligence();
        case Combatant::Ability_Wisdom:
            return getWisdom();
        case Combatant::Ability_Charisma:
            return getCharisma();
        default:
            return -1;
    }
}

qreal BattleDialogModelCombatant::getMoved()
{
    return _moved;
}

void BattleDialogModelCombatant::setMoved(qreal moved)
{
    if(_moved != moved)
    {
        _moved = moved;
        emit moveUpdated();
    }
}

void BattleDialogModelCombatant::incrementMoved(qreal moved)
{
    if(moved != 0.0)
    {
        _moved += moved;
        emit moveUpdated();
    }
}

void BattleDialogModelCombatant::resetMoved()
{
    if(_moved != 0)
    {
        _moved = 0;
        emit moveUpdated();
    }
}

void BattleDialogModelCombatant::setShown(bool isShown)
{
    if(_isShown != isShown)
    {
        _isShown = isShown;
        emit visibilityChanged();
    }
}

void BattleDialogModelCombatant::setKnown(bool isKnown)
{
    if(_isKnown != isKnown)
    {
        _isKnown = isKnown;
        emit visibilityChanged();
    }
}

void BattleDialogModelCombatant::setSelected(bool isSelected)
{
    if(_isSelected != isSelected)
    {
        _isSelected = isSelected;
        emit combatantSelected(this);
    }
}

QDomElement BattleDialogModelCombatant::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("battlecombatant");
}

void BattleDialogModelCombatant::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("combatantId", getCombatant() ? getCombatant()->getID().toString() : QUuid().toString());
    element.setAttribute("type", getCombatantType());
    element.setAttribute("initiative", _initiative);
    element.setAttribute("isShown", _isShown);
    element.setAttribute("isKnown", _isKnown);

    BattleDialogModelObject::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool BattleDialogModelCombatant::belongsToObject(QDomElement& element)
{
    Q_UNUSED(element);

    // Don't auto-input any child objects of the battle. The battle will handle this itself.
    return true;
}

void BattleDialogModelCombatant::setCombatant(Combatant* combatant)
{
    _combatant = combatant;
}
