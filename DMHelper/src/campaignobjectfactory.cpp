#include "campaignobjectfactory.h"
#include "campaignobjectbase.h"
#include "objectfactory.h"
#include "encounterfactory.h"
#include "audiofactory.h"
#include "combatantfactory.h"
#include "mapfactory.h"
#include "placeholder.h"
#include <QDomElement>
#include <QDebug>

CampaignObjectFactory* CampaignObjectFactory::_instance = nullptr;

CampaignObjectFactory::CampaignObjectFactory(QObject *parent) :
    QObject(parent),
    _factoryList()
{
    _factoryList.append(new EncounterFactory());
    _factoryList.append(new AudioFactory());
    _factoryList.append(CombatantFactory::Instance());
    _factoryList.append(new MapFactory());
}

CampaignObjectFactory::~CampaignObjectFactory()
{
    if(!CombatantFactory::Instance())
    {
        qDebug() << "[CampaignObjectFactory] WARNING: Unable to find the CombatantFactory to remove, likely source of crash in CampaignObjectFactory!";
    }
    else
    {
        _factoryList.removeAll(CombatantFactory::Instance());
        CombatantFactory::Shutdown();
    }

    qDeleteAll(_factoryList);
}


CampaignObjectFactory* CampaignObjectFactory::Instance()
{
    if(!_instance)
        _instance = new CampaignObjectFactory;

    return _instance;
}

void CampaignObjectFactory::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

void CampaignObjectFactory::configureFactories(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion)
{
    CampaignObjectFactory* factory = CampaignObjectFactory::Instance();
    if(!factory)
        return;

    for(int i = 0; i < factory->factoryCount(); ++i)
    {
        if(factory->getFactory(i))
            factory->getFactory(i)->configureFactory(ruleset, inputMajorVersion, inputMinorVersion);
    }
}

CampaignObjectBase* CampaignObjectFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    CampaignObjectFactory* factory = CampaignObjectFactory::Instance();
    if(!factory)
        return nullptr;

    for(int i = 0; i < factory->factoryCount(); ++i)
    {
        if(factory->getFactory(i))
        {
            CampaignObjectBase* newObject = factory->getFactory(i)->createObject(objectType, subType, objectName, isImport);
            if(newObject)
                return newObject;
        }
    }

    qDebug() << "[CampaignObjectFactory] ERROR: Unable to create requested object: " << objectType << ", " << subType << ": " << objectName;
    return nullptr;
}

CampaignObjectBase* CampaignObjectFactory::createObject(const QDomElement& element, bool isImport)
{
    CampaignObjectFactory* factory = CampaignObjectFactory::Instance();
    if(!factory)
        return nullptr;

    CampaignObjectBase* result = factory->localCreateObject(element, isImport);
    if(result)
        result->inputXML(element, isImport);

    return result;
}

int CampaignObjectFactory::factoryCount() const
{
    return _factoryList.count();
}

ObjectFactory* CampaignObjectFactory::getFactory(int index) const
{
    if((index < 0) || index >= _factoryList.count())
        return nullptr;
    else
        return _factoryList.at(index);
}

CampaignObjectBase* CampaignObjectFactory::localCreateObject(const QDomElement& element, bool isImport)
{
    for(int i = 0; i < factoryCount(); ++i)
    {
        if(getFactory(i))
        {
            CampaignObjectBase* newObject = getFactory(i)->createObject(element, isImport);
            if(newObject)
                return newObject;
        }
    }

    return new Placeholder;
}
