#include "combatantreference.h"
#include "campaign.h"
#include "character.h"
#include <QDomElement>

CombatantReference::CombatantReference(const QString& name, QObject *parent) :
    Combatant(name, parent),
    _referenceId(),
    _referenceIntId(DMH_GLOBAL_INVALID_ID)
{
}

CombatantReference::CombatantReference(const Combatant &combatant, QObject *parent) :
    Combatant(combatant.getName(), parent),
    _referenceId(combatant.getID()),
    _referenceIntId(combatant.getIntID())
{
}

CombatantReference::CombatantReference(QUuid combatantId, QObject *parent) :
   Combatant(QString(), parent),
   _referenceId(combatantId),
   _referenceIntId(DMH_GLOBAL_INVALID_ID)
{
}

/*
CombatantReference::CombatantReference(const CombatantReference &obj) :
    Combatant(obj),
    _referenceId(obj._referenceId),
    _referenceIntId(obj._referenceIntId)
{
}
*/

Combatant* CombatantReference::getReference()
{
    QUuid referenceId = getReferenceId();
    if(referenceId.isNull())
        return nullptr;

    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
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

    const Campaign* campaign = dynamic_cast<const Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
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
    CombatantReference* newReference = new CombatantReference(getName());

    newReference->copyValues(this);

    newReference->_referenceId = _referenceId;
    newReference->_referenceIntId = _referenceIntId;

    return newReference;
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

    _referenceId = parseIdString(element.attribute("referenceId"), &_referenceIntId);
    Combatant::inputXML(element, isImport);

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

int CombatantReference::getCombatantType() const
{
    return DMHelper::CombatantType_Reference;
}

void CombatantReference::setReference(const Combatant& combatant)
{
    if((_referenceId != combatant.getID()) ||
       ((_referenceId.isNull()) && (_referenceIntId != combatant.getIntID())))
    {
        _referenceId = combatant.getID();
        _referenceIntId = combatant.getIntID();
        emit referenceChanged();
    }
}

void CombatantReference::setReference(const QUuid& combatantId)
{
    if(_referenceId != combatantId)
    {
        _referenceId = combatantId;
        emit referenceChanged();
    }
}

void CombatantReference::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("referenceId", getReferenceId().toString());
    Combatant::internalOutputXML(doc, element, targetDirectory, isExport);
}

