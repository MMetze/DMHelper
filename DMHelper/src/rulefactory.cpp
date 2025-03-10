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

RuleFactory* RuleFactory::_instance = nullptr;

RuleFactory::RuleFactory(const QString& rulesetFile, QObject *parent) :
    QObject{parent},
    _rulesetDir(),
    _defaultBestiary(),
    _rulesetTemplates()
{
    readRuleset(rulesetFile);
}

RuleFactory* RuleFactory::Instance()
{
    return _instance;
}

void RuleFactory::Initialize(const QString& rulesetFile)
{
    if(_instance)
        return;

    qDebug() << "[RuleFactory] Initializing RuleFactory";
    _instance = new RuleFactory(rulesetFile);
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
    return _rulesetTemplates.value(rulesetName);
}

QDir RuleFactory::getRulesetDir() const
{
    return _rulesetDir;
}

void RuleFactory::setDefaultBestiary(const QString& bestiaryFile)
{
    _defaultBestiary = bestiaryFile;
}

QString RuleFactory::getDefaultBestiary() const
{
    return _defaultBestiary;
}

void RuleFactory::readRuleset(const QString& rulesetFile)
{
    qDebug() << "[RuleFactory] Reading ruleset...";

    QDomDocument doc("DMHelperDataXML");
    QFile file(rulesetFile);
    qDebug() << "[RuleFactory] Ruleset file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[RuleFactory] ERROR: Unable to read ruleset file: " << rulesetFile;
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

    _rulesetTemplates.clear();

    QDomElement rulesetElement = root.firstChildElement(QString("ruleset"));
    while(!rulesetElement.isNull())
    {
        RulesetTemplate newRuleset;
        newRuleset._name = rulesetElement.attribute(QString("name"));
        newRuleset._initiative = rulesetElement.attribute(QString("initiative"));
        newRuleset._characterData = rulesetElement.attribute(QString("characterdata"));
        newRuleset._characterUI = rulesetElement.attribute(QString("characterui"));
        newRuleset._monsterData = rulesetElement.attribute(QString("monsterdata"));
        newRuleset._monsterUI = rulesetElement.attribute(QString("monsterui"));
        newRuleset._bestiary = rulesetElement.attribute(QString("bestiary"));

        _rulesetTemplates.insert(newRuleset._name, newRuleset);

        rulesetElement = rulesetElement.nextSiblingElement(QString("ruleset"));
    }

    _rulesetDir = QFileInfo(rulesetFile).absolutePath();

    qDebug() << "[RuleFactory] Completed reading ruleset. Read " << _rulesetTemplates.size() << " rulesets.";
}
