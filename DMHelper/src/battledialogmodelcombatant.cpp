#include "battledialogmodelcombatant.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelCombatant::BattleDialogModelCombatant() :
    CampaignObjectBase(),
    _combatant(nullptr),
    _initiative(0),
    _position(0,0)
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant) :
    CampaignObjectBase(),
    _combatant(combatant),
    _initiative(0),
    _position(0,0)
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant, int initiative, const QPointF& position) :
    CampaignObjectBase(),
    _combatant(combatant),
    _initiative(initiative),
    _position(position)
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(const BattleDialogModelCombatant& other) :
    CampaignObjectBase(),
    _combatant(other._combatant),
    _initiative(other._initiative),
    _position(other._position)
{
}

BattleDialogModelCombatant::~BattleDialogModelCombatant()
{
}

void BattleDialogModelCombatant::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement element = doc.createElement( "battlecombatant" );

    CampaignObjectBase::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("combatantId", getCombatant() ? getCombatant()->getID().toString() : QUuid().toString());
    element.setAttribute("type", getType() );
    element.setAttribute("initiative", _initiative);
    element.setAttribute("positionX", _position.x());
    element.setAttribute("positionY", _position.y());

    internalOutputXML(doc, element, targetDirectory, isExport);

    parent.appendChild(element);
}

void BattleDialogModelCombatant::inputXML(const QDomElement &element, bool isImport)
{
    CampaignObjectBase::inputXML(element, isImport);

    _initiative = element.attribute("initiative",QString::number(0)).toInt();
    _position = QPointF(element.attribute("positionX",QString::number(0)).toDouble(),
                        element.attribute("positionY",QString::number(0)).toDouble());
}

bool BattleDialogModelCombatant::getShown() const
{
    return true;
}

bool BattleDialogModelCombatant::getKnown() const
{
    return true;
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

void BattleDialogModelCombatant::setCombatant(Combatant* combatant)
{
    _combatant = combatant;
}
