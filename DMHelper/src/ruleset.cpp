#include "ruleset.h"
#include "dmconstants.h"
#include "ruleinitiative.h"
#include "rulefactory.h"
#include <QDomDocument>
#include <QDomElement>

const char* DEFAULT_CHARACTER_DATA = "character5e.xml";
const char* DEFAULT_CHARACTER_UI = "./ui/character5e.ui";
const char* DEFAULT_BESTIARY = "DMHelperBestiary.xml";
const char* DEFAULT_MONSTER_DATA = "monster5e.xml";
const char* DEFAULT_MONSTER_UI = "./ui/monster5e.ui";
bool DEFAULT_CHARACTER_DONE_CHECKBOX = true;

Ruleset::Ruleset(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _ruleInitiative(nullptr),
    _characterDataFile(DEFAULT_CHARACTER_DATA),
    _characterUIFile(DEFAULT_CHARACTER_UI),
    _bestiaryFile(DEFAULT_BESTIARY),
    _monsterDataFile(DEFAULT_MONSTER_DATA),
    _monsterUIFile(DEFAULT_MONSTER_UI),
    _combatantDoneCheckbox(DEFAULT_CHARACTER_DONE_CHECKBOX)
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
    _bestiaryFile = element.attribute("bestiary", DEFAULT_BESTIARY);
    _characterDataFile = element.attribute("monsterData", DEFAULT_MONSTER_DATA);
    _characterUIFile = element.attribute("monsterUI", DEFAULT_MONSTER_UI);
    _combatantDoneCheckbox = static_cast<bool>(element.attribute("done", QString::number(DEFAULT_CHARACTER_DONE_CHECKBOX)).toInt());

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

QString Ruleset::getBestiaryFile() const
{
    return _bestiaryFile;
}

QString Ruleset::getMonsterDataFile() const
{
    return _monsterDataFile;
}

QString Ruleset::getMonsterUIFile() const
{
    return _monsterUIFile;
}

bool Ruleset::getCombatantDoneCheckbox() const
{
    return _combatantDoneCheckbox;
}

void Ruleset::setDefaultValues()
{
    _ruleInitiative = RuleFactory::createRuleInitiative(RuleFactory::getRuleInitiativeDefault(), this);
    _characterDataFile = DEFAULT_CHARACTER_DATA;
    _characterUIFile = DEFAULT_CHARACTER_UI;
    _bestiaryFile = DEFAULT_BESTIARY;
    _monsterDataFile = DEFAULT_MONSTER_DATA;
    _monsterUIFile = DEFAULT_MONSTER_UI;
    _combatantDoneCheckbox = DEFAULT_CHARACTER_DONE_CHECKBOX;
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

void Ruleset::setBestiaryFile(const QString& bestiaryFile)
{
    if(_bestiaryFile == bestiaryFile)
        return;

    _bestiaryFile = bestiaryFile;
    emit dirty();
    emit bestiaryFileChanged(_bestiaryFile);
}

void Ruleset::setMonsterDataFile(const QString& monsterDataFile)
{
    if(_monsterDataFile == monsterDataFile)
        return;

    _monsterDataFile = monsterDataFile;
    emit dirty();
    emit monsterDataFileChanged(_monsterDataFile);
}

void Ruleset::setMonsterUIFile(const QString& monsterUIFile)
{
    if(_monsterUIFile == monsterUIFile)
        return;

    _monsterUIFile = monsterUIFile;
    emit dirty();
    emit monsterUIFileChanged(_monsterUIFile);
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

    if(_bestiaryFile != DEFAULT_BESTIARY)
        element.setAttribute("bestiary", _bestiaryFile);

    if(_monsterDataFile != DEFAULT_MONSTER_DATA)
        element.setAttribute("monsterData", _monsterDataFile);

    if(_monsterUIFile != DEFAULT_MONSTER_UI)
        element.setAttribute("monsterUI", _monsterUIFile);

    if(!_combatantDoneCheckbox)
        element.setAttribute("done", _combatantDoneCheckbox);
}
