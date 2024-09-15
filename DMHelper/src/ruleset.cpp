#include "ruleset.h"
#include "dmconstants.h"
#include "ruleinitiative.h"
#include "rulefactory.h"
#include <QDomDocument>
#include <QDomElement>

const char* DEFAULT_CHARACTER_DATA = "character5e.xml";
const char* DEFAULT_CHARACTER_UI = "./ui/character5e.ui";

Ruleset::Ruleset(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _ruleInitiative(nullptr),
    _characterDataFile(),
    _characterUIFile(),
    _combatantDoneCheckbox(false)
{
}

Ruleset::~Ruleset()
{
}

void Ruleset::inputXML(const QDomElement &element, bool isImport)
{
    QString initiativeType = element.attribute("initiative", RuleFactory::getRuleInitiativeDefault());
    _ruleInitiative = RuleFactory::createRuleInitiative(initiativeType, this);

    _characterDataFile = element.attribute("characterData", DEFAULT_CHARACTER_DATA);
    _characterUIFile = element.attribute("characterUI", DEFAULT_CHARACTER_UI);

    _combatantDoneCheckbox = static_cast<bool>(element.attribute("done", QString::number(1)).toInt());

    CampaignObjectBase::inputXML(element, isImport);
}

int Ruleset::getObjectType() const
{
    return DMHelper::CampaignType_Ruleset;
}

bool Ruleset::isTreeVisible() const
{
    return false;
}

RuleInitiative* Ruleset::getRuleInitiative()
{
    if(!_ruleInitiative)
        _ruleInitiative = RuleFactory::createRuleInitiative(RuleFactory::getRuleInitiativeDefault(), this);

    return _ruleInitiative;
}

QString Ruleset::getRuleInitiativeType()
{
    if(!_ruleInitiative)
        _ruleInitiative = RuleFactory::createRuleInitiative(RuleFactory::getRuleInitiativeDefault(), this);

    return _ruleInitiative ? _ruleInitiative->getInitiativeType() : QString();
}

QString Ruleset::getCharacterDataFile() const
{
    return _characterDataFile;
}

QString Ruleset::getCharacterUIFile() const
{
    return _characterUIFile;
}

bool Ruleset::getCombatantDoneCheckbox() const
{
    return _combatantDoneCheckbox;
}

void Ruleset::setDefaultValues()
{
    remove
    QString initiativeType = element.attribute("initiative", RuleFactory::getRuleInitiativeDefault());
    _ruleInitiative = RuleFactory::createRuleInitiative(initiativeType, this);

    _characterDataFile = element.attribute("characterData", DEFAULT_CHARACTER_DATA);
    _characterUIFile = element.attribute("characterUI", DEFAULT_CHARACTER_UI);

    _combatantDoneCheckbox = static_cast<bool>(element.attribute("done", QString::number(1)).toInt());
}

void Ruleset::setRuleInitiative(const QString& initiativeType)
{
    if((_ruleInitiative) && (_ruleInitiative->getInitiativeType() == initiativeType))
        return;

    delete _ruleInitiative;

    _ruleInitiative = RuleFactory::createRuleInitiative(initiativeType, this);
    emit dirty();
    emit initiativeRuleChanged();
}

void Ruleset::setCharacterDataFile(const QString& characterDataFile)
{
    if(_characterDataFile == characterDataFile)
        return;

    _characterDataFile = characterDataFile;
    emit dirty();
    emit characterDataFileChanged(_characterDataFile);
}

void Ruleset::setCharacterUIFile(const QString& characterUIFile)
{
    if(_characterUIFile == characterUIFile)
        return;

    _characterUIFile = characterUIFile;
    emit dirty();
    emit characterUIFileChanged(_characterUIFile);
}

void Ruleset::setCombatantDoneCheckbox(bool checked)
{
    if(_combatantDoneCheckbox == checked)
        return;

    _combatantDoneCheckbox = checked;
    emit dirty();
    emit initiativeRuleChanged();
}

QDomElement Ruleset::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("ruleset");
}

void Ruleset::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    if((_ruleInitiative) && (_ruleInitiative->getInitiativeType() != RuleFactory::getRuleInitiativeDefault()))
        element.setAttribute("initiative", _ruleInitiative->getInitiativeType());

    if(_characterDataFile != DEFAULT_CHARACTER_DATA)
        element.setAttribute("characterData", _characterDataFile);

    if(_characterUIFile != DEFAULT_CHARACTER_UI)
        element.setAttribute("characterUI", _characterUIFile);

    if(!_combatantDoneCheckbox)
        element.setAttribute("done", _combatantDoneCheckbox);
}
