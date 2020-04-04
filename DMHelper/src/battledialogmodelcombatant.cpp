#include "battledialogmodelcombatant.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelCombatant::BattleDialogModelCombatant(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _combatant(nullptr),
    _initiative(0),
    _position(0,0),
    _isShown(true),
    _isKnown(true)
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant) :
    CampaignObjectBase(),
    _combatant(combatant),
    _initiative(0),
    _position(0,0),
    _isShown(true),
    _isKnown(true)
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant, int initiative, const QPointF& position) :
    CampaignObjectBase(),
    _combatant(combatant),
    _initiative(initiative),
    _position(position),
    _isShown(true),
    _isKnown(true)
{
}

/*
BattleDialogModelCombatant::BattleDialogModelCombatant(const BattleDialogModelCombatant& other) :
    CampaignObjectBase(),
    _combatant(other._combatant),
    _initiative(other._initiative),
    _position(other._position),
    _isShown(other._isShown),
    _isKnown(other._isKnown)
{
}
*/

BattleDialogModelCombatant::~BattleDialogModelCombatant()
{
}

/*
void BattleDialogModelCombatant::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement element = doc.createElement("battlecombatant");

    CampaignObjectBase::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("combatantId", getCombatant() ? getCombatant()->getID().toString() : QUuid().toString());
    element.setAttribute("type", getType() );
    element.setAttribute("initiative", _initiative);
    element.setAttribute("positionX", _position.x());
    element.setAttribute("positionY", _position.y());
    element.setAttribute("isShown", _isShown);
    element.setAttribute("isKnown", _isKnown);

    internalOutputXML(doc, element, targetDirectory, isExport);

    parent.appendChild(element);
}
*/

void BattleDialogModelCombatant::inputXML(const QDomElement &element, bool isImport)
{
    CampaignObjectBase::inputXML(element, isImport);

    _initiative = element.attribute("initiative",QString::number(0)).toInt();
    _position = QPointF(element.attribute("positionX",QString::number(0)).toDouble(),
                        element.attribute("positionY",QString::number(0)).toDouble());
    _isShown = static_cast<bool>(element.attribute("isShown",QString::number(1)).toInt());
    _isKnown = static_cast<bool>(element.attribute("isKnown",QString::number(1)).toInt());
}

bool BattleDialogModelCombatant::getShown() const
{
    return _isShown;
}

bool BattleDialogModelCombatant::getKnown() const
{
    return _isKnown;
}

int BattleDialogModelCombatant::getInitiative() const
{
    return _initiative;
}

void BattleDialogModelCombatant::setInitiative(int initiative)
{
    _initiative = initiative;
}

const QPointF& BattleDialogModelCombatant::getPosition() const
{
    return _position;
}

void BattleDialogModelCombatant::setPosition(const QPointF& position)
{
    _position = position;
    emit combatantMoved(this);
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

void BattleDialogModelCombatant::setShown(bool isShown)
{
    _isShown = isShown;
}

void BattleDialogModelCombatant::setKnown(bool isKnown)
{
    _isKnown = isKnown;
}

QDomElement BattleDialogModelCombatant::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("battlecombatant");
}

void BattleDialogModelCombatant::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("combatantId", getCombatant() ? getCombatant()->getID().toString() : QUuid().toString());
    element.setAttribute("type", getCombatantType() );
    element.setAttribute("initiative", _initiative);
    element.setAttribute("positionX", _position.x());
    element.setAttribute("positionY", _position.y());
    element.setAttribute("isShown", _isShown);
    element.setAttribute("isKnown", _isKnown);

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
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

void BattleDialogModelCombatant::copyValues(const BattleDialogModelCombatant &other)
{
    _combatant = other._combatant;
    _initiative = other._initiative;
    _position = other._position;
    _isShown = other._isShown;
    _isKnown = other._isKnown;
}
