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

RuleFactory* RuleFactory::_instance = nullptr;

RuleFactory::RuleFactory(const QString& rulesetFile, QObject *parent) :
    QObject{parent},
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

RuleFactory::RulesetTemplate RuleFactory::getRulesetTemplate(const QString& rulesetName) const
{
    return _rulesetTemplates.value(rulesetName);
}

void RuleFactory::readRuleset(const QString& rulesetFile)
{
    qDebug() << "[RuleFactory] Reading ruleset...";

    QDomDocument doc("DMHelperDataXML");
    QFile file(rulesetFile);
    qDebug() << "[RuleFactory] Ruleset file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[RuleFactory] Unable to read ruleset file: " << rulesetFile;
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
        qDebug() << "[RuleFactory] Unable to parse the ruleset file.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[RuleFactory] Unable to find the root element in the ruleset file.";
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

        _rulesetTemplates.insert(newRuleset._name, newRuleset);

        rulesetElement = rulesetElement.nextSiblingElement(QString("ruleset"));
    }

    qDebug() << "[RuleFActory] Completed reading ruleset.";
}
