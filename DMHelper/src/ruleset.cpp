#include "ruleset.h"
#include "dmconstants.h"
#include "ruleinitiative.h"
#include "rulefactory.h"
#include <QDomDocument>
#include <QDomElement>

Ruleset::Ruleset(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _ruleInitiative(nullptr),
    _characterDataFile(),
    _characterUIFile(),
    _bestiaryFile(),
    _monsterDataFile(),
    _monsterUIFile(),
    _combatantDoneCheckbox()
{
}

Ruleset::Ruleset(const RuleFactory::RulesetTemplate& rulesetTemplate, QObject *parent) :
    CampaignObjectBase(rulesetTemplate._name, parent),
    _ruleInitiative(nullptr),
    _characterDataFile(),
    _characterUIFile(),
    _bestiaryFile(),
    _monsterDataFile(),
    _monsterUIFile(),
    _combatantDoneCheckbox()
{
    setValues(rulesetTemplate);
}

Ruleset::~Ruleset()
{
}

void Ruleset::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    if(!RuleFactory::Instance())
    {
        qDebug("[Ruleset] ERROR: RuleFactory not initialized, not able to read the ruleset file!");
        return;
    }

    QString rulesetName = element.attribute("name", RuleFactory::DEFAULT_RULESET_NAME);
    if(!RuleFactory::Instance()->rulesetExists(rulesetName))
    {
        qDebug() << "[Ruleset] WARNING: The campaign's ruleset template does not exist in DMHelper's ruleset.xml: " << rulesetName << ", using the default ruleset";
        rulesetName = RuleFactory::DEFAULT_RULESET_NAME;
    }

    qDebug() << "[Ruleset] Loading the campaign ruleset based on the template: " << rulesetName;
    setObjectName(rulesetName);

    RuleFactory::RulesetTemplate rulesetTemplate = RuleFactory::Instance()->getRulesetTemplate(objectName());

    QString initiativeType = element.attribute("initiative", rulesetTemplate._initiative);
    _ruleInitiative = RuleFactory::createRuleInitiative(initiativeType, this);

    _characterDataFile = element.attribute("characterData");
    if(_characterDataFile.isEmpty())
        _characterDataFile = RuleFactory::Instance()->getRulesetDir().absoluteFilePath(rulesetTemplate._characterData);

    _characterUIFile = element.attribute("characterUI");
    if(_characterUIFile.isEmpty())
        _characterUIFile = RuleFactory::Instance()->getRulesetDir().absoluteFilePath(rulesetTemplate._characterUI);

    _monsterDataFile = element.attribute("monsterData");
    if(_monsterDataFile.isEmpty())
        _monsterDataFile = RuleFactory::Instance()->getRulesetDir().absoluteFilePath(rulesetTemplate._monsterData);

    _monsterUIFile = element.attribute("monsterUI");
    if(_monsterUIFile.isEmpty())
        _monsterUIFile = RuleFactory::Instance()->getRulesetDir().absoluteFilePath(rulesetTemplate._monsterUI);

    _bestiaryFile = element.attribute("bestiary");
    if(_bestiaryFile.isEmpty())
        _bestiaryFile = RuleFactory::Instance()->getRulesetDir().absoluteFilePath(rulesetTemplate._bestiary);

    _combatantDoneCheckbox = element.hasAttribute("combatantDone") ? static_cast<bool>(element.attribute("combatantDone").toInt()) : rulesetTemplate._combatantDone;
}

int Ruleset::getObjectType() const
{
    return DMHelper::CampaignType_Ruleset;
}

bool Ruleset::isTreeVisible() const
{
    return false;
}

void Ruleset::setValues(const RuleFactory::RulesetTemplate& rulesetTemplate)
{
    if(!RuleFactory::Instance())
    {
        qDebug() << "[Ruleset] ERROR: RuleFactory not initialized, not able to set the ruleset values!";
        return;
    }

    setObjectName(rulesetTemplate._name);

    delete _ruleInitiative;
    _ruleInitiative = RuleFactory::createRuleInitiative(rulesetTemplate._initiative, this);

    QDir rulesetDir = RuleFactory::Instance()->getRulesetDir();

    _characterDataFile = rulesetDir.absoluteFilePath(rulesetTemplate._characterData);
    _characterUIFile = rulesetDir.absoluteFilePath(rulesetTemplate._characterUI);
    _bestiaryFile = rulesetDir.absoluteFilePath(rulesetTemplate._bestiary);
    _monsterDataFile = rulesetDir.absoluteFilePath(rulesetTemplate._monsterData);
    _monsterUIFile = rulesetDir.absoluteFilePath(rulesetTemplate._monsterUI);

    _combatantDoneCheckbox = rulesetTemplate._combatantDone;

    qDebug() << "[Ruleset] Values for the ruleset set to the default values for the template: " << rulesetTemplate._name;
}

bool Ruleset::isInitialized() const
{
    return _ruleInitiative != nullptr;
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
    Q_UNUSED(isExport);

    if(!RuleFactory::Instance())
    {
        qDebug() << "[Ruleset] Error: RuleFactory instance is null, not able to output the ruleset!";
        return;
    }

    element.setAttribute("name", objectName());

    // targetDirectory is the location of the campaign file
    // If the individual files are the same as from the ruleset, they are not added to the campaign file, they will be loaded from the RuleFactory as relative paths to the ruleset
    // If the files are different, they are stored, and filenames made relative to the campaign file

    RuleFactory::RulesetTemplate rulesetTemplate = RuleFactory::Instance()->getRulesetTemplate(objectName());
    QDir rulesetDir = RuleFactory::Instance()->getRulesetDir();

    if((_ruleInitiative) && (_ruleInitiative->getInitiativeType() != rulesetTemplate._initiative))
        element.setAttribute("initiative", _ruleInitiative->getInitiativeType());

    if(!areSameFile(_characterDataFile, rulesetDir.absoluteFilePath(rulesetTemplate._characterData)))
        element.setAttribute("characterData", targetDirectory.relativeFilePath(_characterDataFile));

    if(!areSameFile(_characterUIFile, rulesetDir.absoluteFilePath(rulesetTemplate._characterUI)))
        element.setAttribute("characterUI", targetDirectory.relativeFilePath(_characterUIFile));

    if(!areSameFile(_bestiaryFile, rulesetDir.absoluteFilePath(rulesetTemplate._bestiary)))
        element.setAttribute("bestiary", targetDirectory.relativeFilePath(_bestiaryFile));

    if(!areSameFile(_monsterDataFile, rulesetDir.absoluteFilePath(rulesetTemplate._monsterData)))
        element.setAttribute("monsterData", targetDirectory.relativeFilePath(_monsterDataFile));

    if(!areSameFile(_monsterUIFile, rulesetDir.absoluteFilePath(rulesetTemplate._monsterUI)))
        element.setAttribute("monsterUI", targetDirectory.relativeFilePath(_monsterUIFile));

    if(_combatantDoneCheckbox != rulesetTemplate._combatantDone)
        element.setAttribute("combatantDone", _combatantDoneCheckbox);
}

bool Ruleset::areSameFile(const QString &file1, const QString &file2) const
{
    QFileInfo fileInfo1(file1);
    QFileInfo fileInfo2(file2);

    QString canonicalPath1 = fileInfo1.canonicalFilePath();
    QString canonicalPath2 = fileInfo2.canonicalFilePath();

    return ((!canonicalPath1.isEmpty()) && (canonicalPath1 == canonicalPath2));
}
