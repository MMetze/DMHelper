#include "monsterfactory.h"
#include "ruleset.h"
#include <QDebug>

MonsterFactory* MonsterFactory::_instance = nullptr;

MonsterFactory::MonsterFactory(QObject *parent) :
    TemplateFactory{parent},
    _compatibilityMode(false)
{
}

MonsterFactory* MonsterFactory::Instance()
{
    if(!_instance)
    {
        qDebug() << "[MonsterFactory] Initializing Monster Factory";
        _instance = new MonsterFactory();
    }

    return _instance;
}

void MonsterFactory::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

CampaignObjectBase* MonsterFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(objectType);
    Q_UNUSED(subType);
    Q_UNUSED(isImport);
    Q_UNUSED(objectName);

    return nullptr;
}

CampaignObjectBase* MonsterFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(element);
    Q_UNUSED(isImport);

    return nullptr;
}

void MonsterFactory::configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion)
{
    loadTemplate(ruleset.getMonsterDataFile());
    connect(&ruleset, &Ruleset::monsterDataFileChanged, this, &MonsterFactory::loadTemplate);

    _compatibilityMode = (inputMajorVersion < 2) || ((inputMajorVersion == 2) && (inputMinorVersion < 2));

    should this now load the bestiary?
}
