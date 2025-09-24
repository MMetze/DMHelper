#include "rulefactory.h"
#include "ruleinitiative2e.h"
#include "ruleinitiative5e.h"
#include "ruleinitiativegroup.h"
#include "ruleinitiativegroupmonsters.h"
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

const char* RuleFactory::DEFAULT_RULESET_NAME = "DnD 5e 2014";
const char* DEFAULT_CHARACTER_DATA = "character5e.xml";
const char* DEFAULT_CHARACTER_UI = "./ui/character5e.ui";
const char* DEFAULT_MONSTER_DATA = "monster5e.xml";
const char* DEFAULT_MONSTER_UI = "./ui/monster5e.ui";
const char* DEFAULT_BESTIARY = "DMHelperBestiary.xml";
bool DEFAULT_CHARACTER_DONE = true;
const char* DEFAULT_MOVEMENT = "distance";

RuleFactory* RuleFactory::_instance = nullptr;

RuleFactory::RuleFactory(const QString& defaultRulesetFile, const QString& userRulesetFile, QObject *parent) :
    QObject{parent},
    _defaultBestiary(),
    _rulesetTemplates()
{    
    _rulesetTemplates.clear();
    readRuleset(defaultRulesetFile);
    readRuleset(userRulesetFile);
}

RuleFactory* RuleFactory::Instance()
{
    return _instance;
}

void RuleFactory::Initialize(const QString& defaultRulesetFile, const QString& userRulesetFile)
{
    if(_instance)
        return;

    qDebug() << "[RuleFactory] Initializing RuleFactory with default ruleset file: " << defaultRulesetFile << " and user ruleset file: " << userRulesetFile;
    _instance = new RuleFactory(defaultRulesetFile, userRulesetFile);
}

void RuleFactory::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

RuleInitiative* RuleFactory::createRuleInitiative(const QString& ruleInitiativeType, QObject *parent)
{
    if(ruleInitiativeType == RuleInitiative5e::InitiativeType)
        return new RuleInitiative5e(parent);

    if(ruleInitiativeType == RuleInitiativeGroup::InitiativeType)
        return new RuleInitiativeGroup(parent);

    if(ruleInitiativeType == RuleInitiativeGroupMonsters::InitiativeType)
        return new RuleInitiativeGroupMonsters(parent);

    if(ruleInitiativeType == RuleInitiative2e::InitiativeType)
        return new RuleInitiative2e(parent);

    return nullptr;
}

QString RuleFactory::getRuleInitiativeDefault()
{
    return RuleInitiative5e::InitiativeType;
}

QStringList RuleFactory::getRuleInitiativeNames()
{
    QStringList result;

    result << RuleInitiative5e::InitiativeType << RuleInitiative5e::InitiativeDescription
           << RuleInitiative2e::InitiativeType << RuleInitiative2e::InitiativeDescription
           << RuleInitiativeGroup::InitiativeType << RuleInitiativeGroup::InitiativeDescription
           << RuleInitiativeGroupMonsters::InitiativeType << RuleInitiativeGroupMonsters::InitiativeDescription;

    return result;
}

QList<QString> RuleFactory::getRulesetNames() const
{
    return _rulesetTemplates.keys();
}

QList<RuleFactory::RulesetTemplate> RuleFactory::getRulesetTemplates() const
{
    return _rulesetTemplates.values();
}

bool RuleFactory::rulesetExists(const QString& rulesetName) const
{
    return ((!rulesetName.isEmpty()) && (_rulesetTemplates.contains(rulesetName)));
}

RuleFactory::RulesetTemplate RuleFactory::getRulesetTemplate(const QString& rulesetName) const
{
    if(rulesetExists(rulesetName))
    {
        return _rulesetTemplates.value(rulesetName);
    }
    else
    {
        qDebug() << "[RuleFactory] WARNING: Requested ruleset " << rulesetName << " does not exist, returning a default ruleset template.";
        return RulesetTemplate(RuleFactory::DEFAULT_RULESET_NAME,
                               RuleInitiative5e::InitiativeType,
                               DEFAULT_CHARACTER_DATA,
                               DEFAULT_CHARACTER_UI,
                               DEFAULT_MONSTER_DATA,
                               DEFAULT_MONSTER_UI,
                               getDefaultBestiary(),
                               QDir(),
                               QString(),
                               DEFAULT_CHARACTER_DONE);
    }
}

void RuleFactory::setDefaultBestiary(const QString& bestiaryFile)
{
    if(_defaultBestiary == bestiaryFile)
        return;

    _defaultBestiary = bestiaryFile;

    if(_rulesetTemplates.contains(DEFAULT_RULESET_NAME))
    {
        RulesetTemplate defaultTemplate = _rulesetTemplates.value(DEFAULT_RULESET_NAME);
        if((defaultTemplate._bestiary == DEFAULT_BESTIARY) && (!_defaultBestiary.isEmpty()))
            defaultTemplate._bestiary = _defaultBestiary;

        _rulesetTemplates.insert(DEFAULT_RULESET_NAME, defaultTemplate);
    }
}

QString RuleFactory::getDefaultBestiary() const
{
    return _defaultBestiary;
}

void RuleFactory::readRuleset(const QString& rulesetFile)
{
    qDebug() << "[RuleFactory] Reading ruleset from " << rulesetFile;

    QDomDocument doc("DMHelperDataXML");
    QFile file(rulesetFile);
    qDebug() << "[RuleFactory] Ruleset file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[RuleFactory] ERROR: Unable to read ruleset file: " << rulesetFile << ", error: " << file.error() << ", " << file.errorString();
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[RuleFactory] ERROR: Unable to parse the ruleset file: " << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[RuleFactory] ERROR: Unable to find the root element in the ruleset file.";
        return;
    }

    QDomElement rulesetElement = root.firstChildElement(QString("ruleset"));
    while(!rulesetElement.isNull())
    {
        RulesetTemplate newRuleset;
        newRuleset._name = rulesetElement.attribute(QString("name"));
        if(!newRuleset._name.isEmpty())
        {
            newRuleset._initiative = rulesetElement.attribute(QString("initiative"));
            newRuleset._characterData = rulesetElement.attribute(QString("characterdata"));
            newRuleset._characterUI = rulesetElement.attribute(QString("characterui"));
            newRuleset._monsterData = rulesetElement.attribute(QString("monsterdata"));
            newRuleset._monsterUI = rulesetElement.attribute(QString("monsterui"));
            newRuleset._bestiary = rulesetElement.attribute(QString("bestiary"));
            newRuleset._rulesetDir = QFileInfo(rulesetFile).absolutePath();
            newRuleset._combatantDone = static_cast<bool>(rulesetElement.attribute(QString("combatantdone")).toInt());
            newRuleset._movement = rulesetElement.attribute(QString("movement"));

            if((newRuleset._name == DEFAULT_RULESET_NAME) && (newRuleset._bestiary == DEFAULT_BESTIARY) && (!getDefaultBestiary().isEmpty()))
                newRuleset._bestiary = getDefaultBestiary();

            if(_rulesetTemplates.contains(newRuleset._name))
                qDebug() << "[RuleFactory] WARNING: Duplicate ruleset name found: " << newRuleset._name << " in ruleset file: " << rulesetFile;
            else
                _rulesetTemplates.insert(newRuleset._name, newRuleset);
        }

        rulesetElement = rulesetElement.nextSiblingElement(QString("ruleset"));
    }

    qDebug() << "[RuleFactory] Completed reading ruleset. Read " << _rulesetTemplates.size() << " rulesets.";
}
