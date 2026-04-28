#include "spellbookfactory.h"
#include "ruleset.h"
#include <QDebug>

SpellbookFactory* SpellbookFactory::_instance = nullptr;

SpellbookFactory::SpellbookFactory(QObject *parent) :
    TemplateFactory{parent},
    _compatibilityMode(false),
    _rulesetName{}
{
}

SpellbookFactory* SpellbookFactory::Instance()
{
    if(!_instance)
    {
        qDebug() << "[SpellbookFactory] Initializing Spellbook Factory";
        _instance = new SpellbookFactory();
    }

    return _instance;
}

void SpellbookFactory::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

QString SpellbookFactory::getRulesetName() const
{
    return _rulesetName;
}

CampaignObjectBase* SpellbookFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(objectType);
    Q_UNUSED(subType);
    Q_UNUSED(objectName);
    Q_UNUSED(isImport);
    return nullptr;
}

CampaignObjectBase* SpellbookFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(element);
    Q_UNUSED(isImport);
    return nullptr;
}

void SpellbookFactory::configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion)
{
    // Resolve via the ruleset's spell data file accessor (added to Ruleset in
    // Phase A4). If not yet set, fall back silently — the factory will simply
    // have no attribute definitions and templates will use default values.
    const QString dataFile = ruleset.getSpellDataFile();
    if(!dataFile.isEmpty())
        loadTemplate(dataFile);

    _compatibilityMode = (inputMajorVersion < 2) || ((inputMajorVersion == 2) && (inputMinorVersion < 2));
    _rulesetName = ruleset.getName();
}
