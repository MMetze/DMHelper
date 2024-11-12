#include "combatantfactory.h"
#include "characterv2.h"
#include "characterv2converter.h"
#include "monster.h"
#include "dmconstants.h"
#include "ruleset.h"
#include "combatantreference.h"
#include <QString>
#include <QDomElement>
#include <QDebug>

CombatantFactory* CombatantFactory::_instance = nullptr;

CombatantFactory::CombatantFactory(QObject *parent) :
    TemplateFactory(parent),
    _compatibilityMode(false)
{
}

CombatantFactory* CombatantFactory::Instance()
{
    if(!_instance)
    {
        qDebug() << "[CombatantFactory] Initializing Combatant Factory";
        _instance = new CombatantFactory();
    }

    return _instance;
}

void CombatantFactory::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

CampaignObjectBase* CombatantFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(isImport);

    if(objectType != DMHelper::CampaignType_Combatant)
    {
        qDebug() << "[CombatantFactory] ERROR: unexpected creation of non-combatant object type: " << objectType << ", subType: " << subType;
        return nullptr;
    }

    switch(subType)
    {
        case DMHelper::CombatantType_Character:
            return setDefaultValues(new Characterv2(objectName));
        case DMHelper::CombatantType_Reference:
            return new CombatantReference();
        case DMHelper::CombatantType_Base:
        case DMHelper::CombatantType_Monster:
        default:
            return nullptr;
    }
}

CampaignObjectBase* CombatantFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.tagName() != QString("combatant"))
        return nullptr;

    bool ok = false;
    int combatantType = element.attribute("type").toInt(&ok);

    if((!ok) || (combatantType != DMHelper::CombatantType_Character))
    {
        qDebug() << "[CombatantFactory] ERROR: unexpected combatant type found reading element: " << combatantType << ", read success: " << ok;
        return nullptr;
    }

    if(_compatibilityMode)
        return setDefaultValues(new Characterv2Converter());
    else
        return setDefaultValues(new Characterv2());
}


void CombatantFactory::configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion)
{
    loadTemplate(ruleset.getCharacterDataFile());
    connect(&ruleset, &Ruleset::characterDataFileChanged, this, &CombatantFactory::loadTemplate);

    _compatibilityMode = (inputMajorVersion < 2) || ((inputMajorVersion == 2) && (inputMinorVersion < 4));
}

CampaignObjectBase* CombatantFactory::setDefaultValues(CampaignObjectBase* object)
{
    if((!object) || (object->getObjectType() != DMHelper::CampaignType_Combatant))
        return object;

    Characterv2* character = dynamic_cast<Characterv2*>(object);
    if(!character)
        return object;

    QHash<QString, DMHAttribute> attributes = getAttributes();
    for(auto it = attributes.begin(); it != attributes.end(); ++it)
    {
        character->setValue(it.key(), it->_default);
    }

    QHash<QString, DMHAttribute> elements = getElements();
    for(auto it = elements.begin(); it != elements.end(); ++it)
    {
        character->setValue(it.key(), it->_default);
    }

    return character;
}
