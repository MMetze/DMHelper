#include "combatantreference.h"
#include "campaign.h"
#include "character.h"
#include <QDomElement>

CombatantReference::CombatantReference(QObject *parent) :
    Combatant(parent),
    _referenceId(DMH_GLOBAL_INVALID_ID)
{
}

CombatantReference::CombatantReference(const Combatant &combatant, QObject *parent) :
    Combatant(parent),
    _referenceId(combatant.getID())
{
}

CombatantReference::CombatantReference(int combatantId, QObject *parent) :
   Combatant(parent),
   _referenceId(combatantId)
{
}

CombatantReference::CombatantReference(const CombatantReference &obj) :
    Combatant(obj),
    _referenceId(obj._referenceId)
{
}

Combatant* CombatantReference::getReference()
{
    if(_referenceId == DMH_GLOBAL_INVALID_ID)
        return nullptr;

    Campaign* campaign = getCampaign();
    if(!campaign)
        return nullptr;

    Character* result = campaign->getCharacterById(_referenceId);
    if(result)
        return result;

    result = campaign->getNPCById(_referenceId);
    if(result)
        return result;

    return nullptr;
}

const Combatant* CombatantReference::getReference() const
{
    if(_referenceId == DMH_GLOBAL_INVALID_ID)
        return nullptr;

    const Campaign* campaign = getCampaign();
    if(!campaign)
        return nullptr;

    const Character* result = campaign->getCharacterById(_referenceId);
    if(result)
        return result;

    result = campaign->getNPCById(_referenceId);
    if(result)
        return result;

    return nullptr;
}

int CombatantReference::getReferenceId() const
{
    return _referenceId;
}

Combatant* CombatantReference::clone() const
{
    return new CombatantReference(*this);
}

int CombatantReference::getSpeed() const
{
    return 0;
}

int CombatantReference::getStrength() const
{
    return 0;
}

int CombatantReference::getDexterity() const
{
    return 0;
}

int CombatantReference::getConstitution() const
{
    return 0;
}

int CombatantReference::getIntelligence() const
{
    return 0;
}

int CombatantReference::getWisdom() const
{
    return 0;
}

int CombatantReference::getCharisma() const
{
    return 0;
}

void CombatantReference::inputXML(const QDomElement &element)
{
    beginBatchChanges();

    Combatant::inputXML(element);

    _referenceId = element.attribute("referenceId").toInt();

    endBatchChanges();
}

QString CombatantReference::getName() const
{
    const Combatant* ref = getReference();
    if(ref)
        return ref->getName();
    else
        return QString("<Bad Reference Object>");
}

int CombatantReference::getType() const
{
    return DMHelper::CombatantType_Reference;
}

void CombatantReference::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);

    element.setAttribute( "referenceId", getReferenceId() );
}

