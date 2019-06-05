#include "combatantreference.h"
#include "campaign.h"
#include "character.h"
#include <QDomElement>

CombatantReference::CombatantReference(QObject *parent) :
    Combatant(parent),
    _referenceId(),
    _referenceIntId(DMH_GLOBAL_INVALID_ID)
{
}

CombatantReference::CombatantReference(const Combatant &combatant, QObject *parent) :
    Combatant(parent),
    _referenceId(combatant.getID()),
    _referenceIntId(combatant.getIntID())
{
}

CombatantReference::CombatantReference(QUuid combatantId, QObject *parent) :
   Combatant(parent),
   _referenceId(combatantId),
   _referenceIntId(DMH_GLOBAL_INVALID_ID)
{
}

CombatantReference::CombatantReference(const CombatantReference &obj) :
    Combatant(obj),
    _referenceId(obj._referenceId),
    _referenceIntId(obj._referenceIntId)
{
}

Combatant* CombatantReference::getReference()
{
    QUuid referenceId = getReferenceId();
    if(referenceId.isNull())
        return nullptr;

    Campaign* campaign = getCampaign();
    if(!campaign)
        return nullptr;

    Character* result = campaign->getCharacterById(referenceId);
    if(result)
        return result;

    result = campaign->getNPCById(referenceId);
    if(result)
        return result;

    return nullptr;
}

const Combatant* CombatantReference::getReference() const
{
    QUuid referenceId = getReferenceId();
    if(referenceId.isNull())
        return nullptr;

    const Campaign* campaign = getCampaign();
    if(!campaign)
        return nullptr;

    const Character* result = campaign->getCharacterById(referenceId);
    if(result)
        return result;

    result = campaign->getNPCById(referenceId);
    if(result)
        return result;

    return nullptr;
}

QUuid CombatantReference::getReferenceId()
{
    if((_referenceId.isNull()) && (_referenceIntId != DMH_GLOBAL_INVALID_ID))
    {
        _referenceId = findUuid(_referenceIntId);
    }

    return _referenceId;
}

QUuid CombatantReference::getReferenceId() const
{
    if((_referenceId.isNull()) && (_referenceIntId != DMH_GLOBAL_INVALID_ID))
    {
        return findUuid(_referenceIntId);
    }
    else
    {
        return _referenceId;
    }
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

void CombatantReference::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();

    Combatant::inputXML(element, isImport);

    _referenceId = parseIdString(element.attribute("referenceId"), &_referenceIntId);

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

void CombatantReference::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("referenceId", getReferenceId().toString());
}

