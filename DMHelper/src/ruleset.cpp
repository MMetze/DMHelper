#include "ruleset.h"
#include "conditions.h"
#include "dmconstants.h"
#include "ruleinitiative.h"
#include "rulefactory.h"
#include <QDomDocument>
#include <QDomElement>

Ruleset::Ruleset(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _ruleInitiative(nullptr),
    _conditions(nullptr),
    _rulesetDefaultConditions(nullptr),
    _conditionsFile(),
    _characterDataFile(),
    _characterUIFile(),
    _bestiaryFile(),
    _monsterDataFile(),
    _monsterUIFile(),
    _spellbookFile(),
    _spellDataFile(),
    _spellUIFile(),
    _combatantUIFile(),
    _combatantFrameUIFile(),
    _combatantFrameDataFile(),
    _lairActionUIFile(),
    _combatantDoneCheckbox(),
    _hitPointsCountDown(true),
    _movementType(DMHelper::MovementType_Distance),
    _movementRanges(),
    _batchProcessing(false),
    _changed(false)
{
}

Ruleset::Ruleset(const RuleFactory::RulesetTemplate& rulesetTemplate, QObject *parent) :
    CampaignObjectBase(rulesetTemplate._name, parent),
    _ruleInitiative(nullptr),
    _conditions(nullptr),
    _rulesetDefaultConditions(nullptr),
    _conditionsFile(),
    _characterDataFile(),
    _characterUIFile(),
    _bestiaryFile(),
    _monsterDataFile(),
    _monsterUIFile(),
    _spellbookFile(),
    _spellDataFile(),
    _spellUIFile(),
    _combatantUIFile(),
    _combatantFrameUIFile(),
    _combatantFrameDataFile(),
    _lairActionUIFile(),
    _combatantDoneCheckbox(),
    _hitPointsCountDown(true),
    _movementType(DMHelper::MovementType_Distance),
    _movementRanges(),
    _batchProcessing(false),
    _changed(false)
{
    setValues(rulesetTemplate);
}

Ruleset::~Ruleset()
{
    delete _conditions;
    delete _rulesetDefaultConditions;
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
        _characterDataFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._characterData);

    _characterUIFile = element.attribute("characterUI");
    if(_characterUIFile.isEmpty())
        _characterUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._characterUI);

    _monsterDataFile = element.attribute("monsterData");
    if(_monsterDataFile.isEmpty())
        _monsterDataFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._monsterData);

    _monsterUIFile = element.attribute("monsterUI");
    if(_monsterUIFile.isEmpty())
        _monsterUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._monsterUI);

    _bestiaryFile = element.attribute("bestiary");
    if(_bestiaryFile.isEmpty())
        _bestiaryFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._bestiary);

    _spellbookFile = element.attribute("spellbook");
    if((_spellbookFile.isEmpty()) && (!rulesetTemplate._spellbook.isEmpty()))
        _spellbookFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellbook);

    _spellDataFile = element.attribute("spelldata");
    if((_spellDataFile.isEmpty()) && (!rulesetTemplate._spellData.isEmpty()))
        _spellDataFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellData);

    _spellUIFile = element.attribute("spellui");
    if((_spellUIFile.isEmpty()) && (!rulesetTemplate._spellUI.isEmpty()))
        _spellUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellUI);

    _combatantUIFile = element.attribute("combatantui");
    if((_combatantUIFile.isEmpty()) && (!rulesetTemplate._combatantUI.isEmpty()))
        _combatantUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantUI);

    _combatantFrameUIFile = element.attribute("combatantframeui");
    if((_combatantFrameUIFile.isEmpty()) && (!rulesetTemplate._combatantFrameUI.isEmpty()))
        _combatantFrameUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantFrameUI);

    _combatantFrameDataFile = element.attribute("combatantframedata");
    if((_combatantFrameDataFile.isEmpty()) && (!rulesetTemplate._combatantFrameData.isEmpty()))
        _combatantFrameDataFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantFrameData);

    _lairActionUIFile = element.attribute("lairactionui");
    if((_lairActionUIFile.isEmpty()) && (!rulesetTemplate._lairActionUI.isEmpty()))
        _lairActionUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._lairActionUI);

    _combatantDoneCheckbox = element.hasAttribute("combatantDone") ? static_cast<bool>(element.attribute("combatantDone").toInt()) : rulesetTemplate._combatantDone;
    _hitPointsCountDown = element.hasAttribute("hitPointsCountDown") ? static_cast<bool>(element.attribute("hitPointsCountDown").toInt()) : rulesetTemplate._hitPointsCountDown;

    setMovementString(element.attribute("movementType"));

    // Load conditions from ruleset template
    delete _conditions;
    _conditions = new Conditions(this);
    delete _rulesetDefaultConditions;
    _rulesetDefaultConditions = new Conditions(this);
    if(!rulesetTemplate._conditionsFile.isEmpty())
    {
        _conditionsFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._conditionsFile);
        _conditions->loadFromFile(_conditionsFile);
        _rulesetDefaultConditions->loadFromFile(_conditionsFile);
    }

    // Apply campaign condition deltas if present
    QDomElement conditionDeltasElement = element.firstChildElement(QStringLiteral("conditionDeltas"));
    if(!conditionDeltasElement.isNull())
    {
        // Resolve custom icon paths relative to the campaign file directory
        // Note: the campaign directory is not directly available here, but the element's
        // ownerDocument was loaded from the campaign file. We use the rulesetDir as fallback
        // since the actual campaign dir will be set by the caller.
        _conditions->applyDeltas(conditionDeltasElement, rulesetTemplate._rulesetDir);
    }
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

    _characterDataFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._characterData);
    _characterUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._characterUI);
    _bestiaryFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._bestiary);
    _monsterDataFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._monsterData);
    _monsterUIFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._monsterUI);

    _spellbookFile = rulesetTemplate._spellbook.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellbook);
    _spellDataFile = rulesetTemplate._spellData.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellData);
    _spellUIFile = rulesetTemplate._spellUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellUI);

    _combatantUIFile = rulesetTemplate._combatantUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantUI);
    _combatantFrameUIFile = rulesetTemplate._combatantFrameUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantFrameUI);
    _combatantFrameDataFile = rulesetTemplate._combatantFrameData.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantFrameData);
    _lairActionUIFile = rulesetTemplate._lairActionUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._lairActionUI);

    _combatantDoneCheckbox = rulesetTemplate._combatantDone;
    _hitPointsCountDown = rulesetTemplate._hitPointsCountDown;

    // Load conditions from ruleset template
    delete _conditions;
    _conditions = new Conditions(this);
    delete _rulesetDefaultConditions;
    _rulesetDefaultConditions = new Conditions(this);
    if(!rulesetTemplate._conditionsFile.isEmpty())
    {
        _conditionsFile = rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._conditionsFile);
        _conditions->loadFromFile(_conditionsFile);
        _rulesetDefaultConditions->loadFromFile(_conditionsFile);
    }

    qDebug() << "[Ruleset] Values for the ruleset set to the default values for the template: " << rulesetTemplate._name;
}

void Ruleset::startBatchProcessing()
{
    _batchProcessing = true;
    _changed = false;
}

void Ruleset::endBatchProcessing()
{
    _batchProcessing = false;
    if(_changed)
    {
        emit rulesetChanged();
        _changed = false;
    }
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

Conditions* Ruleset::getConditions()
{
    if(!_conditions)
    {
        _conditions = new Conditions(const_cast<Ruleset*>(this));
    }
    return _conditions;
}

Conditions* Ruleset::getRulesetDefaultConditions() const
{
    return _rulesetDefaultConditions;
}

QString Ruleset::getConditionsFile() const
{
    return _conditionsFile;
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

QString Ruleset::getSpellbookFile() const
{
    return _spellbookFile;
}

QString Ruleset::getSpellDataFile() const
{
    return _spellDataFile;
}

QString Ruleset::getSpellUIFile() const
{
    return _spellUIFile;
}

QString Ruleset::getCombatantUIFile() const
{
    return _combatantUIFile;
}

QString Ruleset::getCombatantFrameUIFile() const
{
    return _combatantFrameUIFile;
}

QString Ruleset::getCombatantFrameDataFile() const
{
    return _combatantFrameDataFile;
}

QString Ruleset::getLairActionUIFile() const
{
    return _lairActionUIFile;
}

bool Ruleset::getCombatantDoneCheckbox() const
{
    return _combatantDoneCheckbox;
}

QString Ruleset::getMovementString() const
{
    return movementStringFromType(_movementType, &_movementRanges);
}

bool Ruleset::getHitPointsCoundDown() const
{
    return _hitPointsCountDown;
}

DMHelper::MovementType Ruleset::getMovementType() const
{
    return _movementType;
}

QList<int> Ruleset::getMovementRanges() const
{
    return _movementRanges;
}

DMHelper::MovementType Ruleset::movementTypeFromString(const QString& movementStr, QList<int>* movementRanges)
{
    if(movementRanges)
        movementRanges->clear();

    if(movementStr.startsWith(QString("range")))
    {
        if(movementRanges)
        {
            QStringList movementRangeStrings = movementStr.right(movementStr.length() - 6).split(QString(";"));
            for(QString& rangeStr : movementRangeStrings)
            {
                bool ok = false;
                int rangeValue = rangeStr.toInt(&ok);
                if(ok)
                    movementRanges->append(rangeValue);
            }
        }

        return DMHelper::MovementType_Range;
    }
    else if(movementStr.startsWith(QString("none")))
    {
        return DMHelper::MovementType_None;
    }
    else
    {
        return DMHelper::MovementType_Distance;
    }
}

QString Ruleset::movementStringFromType(DMHelper::MovementType movementType, const QList<int>* movementRanges)
{
    switch(movementType)
    {
        case DMHelper::MovementType_Range:
        {
            QString result = QString("range");
            if(movementRanges)
            {
                for(int range : *movementRanges)
                    result.append(QString(";") + QString::number(range));
            }
            return result;
        }
        case DMHelper::MovementType_None:
            return QString("none");
        case DMHelper::MovementType_Distance:
        default:
            return QString("distance");
    }
}

void Ruleset::setRuleInitiative(const QString& initiativeType)
{
    if((_ruleInitiative) && (_ruleInitiative->getInitiativeType() == initiativeType))
        return;

    delete _ruleInitiative;

    _ruleInitiative = RuleFactory::createRuleInitiative(initiativeType, this);
    emit dirty();
    registerChange();
}

void Ruleset::setConditionsFile(const QString& conditionsFile)
{
    if(_conditionsFile == conditionsFile)
        return;

    _conditionsFile = conditionsFile;

    delete _conditions;
    _conditions = new Conditions(this);
    delete _rulesetDefaultConditions;
    _rulesetDefaultConditions = new Conditions(this);
    if(!_conditionsFile.isEmpty())
    {
        _conditions->loadFromFile(_conditionsFile);
        _rulesetDefaultConditions->loadFromFile(_conditionsFile);
    }

    Conditions::setActiveConditions(_conditions);

    emit dirty();
    registerChange();
}

void Ruleset::setCharacterDataFile(const QString& characterDataFile)
{
    if(_characterDataFile == characterDataFile)
        return;

    _characterDataFile = characterDataFile;
    emit dirty();
    registerChange();
}

void Ruleset::setCharacterUIFile(const QString& characterUIFile)
{
    if(_characterUIFile == characterUIFile)
        return;

    _characterUIFile = characterUIFile;
    emit dirty();
    registerChange();
}

void Ruleset::setBestiaryFile(const QString& bestiaryFile)
{
    if(_bestiaryFile == bestiaryFile)
        return;

    _bestiaryFile = bestiaryFile;
    emit dirty();
    registerChange();
}

void Ruleset::setMonsterDataFile(const QString& monsterDataFile)
{
    if(_monsterDataFile == monsterDataFile)
        return;

    _monsterDataFile = monsterDataFile;
    emit dirty();
    registerChange();
}

void Ruleset::setMonsterUIFile(const QString& monsterUIFile)
{
    if(_monsterUIFile == monsterUIFile)
        return;

    _monsterUIFile = monsterUIFile;
    emit dirty();
    registerChange();
}

void Ruleset::setSpellbookFile(const QString& spellbookFile)
{
    if(_spellbookFile == spellbookFile)
        return;

    _spellbookFile = spellbookFile;
    emit dirty();
    registerChange();
}

void Ruleset::setSpellDataFile(const QString& spellDataFile)
{
    if(_spellDataFile == spellDataFile)
        return;

    _spellDataFile = spellDataFile;
    emit dirty();
    registerChange();
}

void Ruleset::setSpellUIFile(const QString& spellUIFile)
{
    if(_spellUIFile == spellUIFile)
        return;

    _spellUIFile = spellUIFile;
    emit dirty();
    registerChange();
}

void Ruleset::setCombatantUIFile(const QString& combatantUIFile)
{
    if(_combatantUIFile == combatantUIFile)
        return;

    _combatantUIFile = combatantUIFile;
    emit dirty();
    registerChange();
}

void Ruleset::setCombatantFrameUIFile(const QString& combatantFrameUIFile)
{
    if(_combatantFrameUIFile == combatantFrameUIFile)
        return;

    _combatantFrameUIFile = combatantFrameUIFile;
    emit dirty();
    registerChange();
}

void Ruleset::setCombatantFrameDataFile(const QString& combatantFrameDataFile)
{
    if(_combatantFrameDataFile == combatantFrameDataFile)
        return;

    _combatantFrameDataFile = combatantFrameDataFile;
    emit dirty();
    registerChange();
}

void Ruleset::setLairActionUIFile(const QString& lairActionUIFile)
{
    if(_lairActionUIFile == lairActionUIFile)
        return;

    _lairActionUIFile = lairActionUIFile;
    emit dirty();
    registerChange();
}

void Ruleset::setCombatantDoneCheckbox(bool checked)
{
    if(_combatantDoneCheckbox == checked)
        return;

    _combatantDoneCheckbox = checked;
    emit dirty();
    registerChange();
}

void Ruleset::setHitPointsCountDown(bool countDown)
{
    if(_hitPointsCountDown == countDown)
        return;

    _hitPointsCountDown = countDown;
    emit dirty();
    registerChange();
}

void Ruleset::setMovementString(const QString& movement)
{
    _movementType = movementTypeFromString(movement, &_movementRanges);
}

void Ruleset::setMovementType(DMHelper::MovementType type)
{
    if(_movementType == type)
        return;

    _movementType = type;
    emit dirty();
}

void Ruleset::setMovementRanges(QList<int> ranges)
{
    if(_movementRanges == ranges)
        return;

    _movementRanges = ranges;
    emit dirty();
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

    if((_ruleInitiative) && (_ruleInitiative->getInitiativeType() != rulesetTemplate._initiative))
        element.setAttribute("initiative", _ruleInitiative->getInitiativeType());

    if(!areSameFile(_characterDataFile, rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._characterData)))
        element.setAttribute("characterData", targetDirectory.relativeFilePath(_characterDataFile));

    if(!areSameFile(_characterUIFile, rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._characterUI)))
        element.setAttribute("characterUI", targetDirectory.relativeFilePath(_characterUIFile));

    if(!areSameFile(_bestiaryFile, rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._bestiary)))
        element.setAttribute("bestiary", targetDirectory.relativeFilePath(_bestiaryFile));

    if(!areSameFile(_monsterDataFile, rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._monsterData)))
        element.setAttribute("monsterData", targetDirectory.relativeFilePath(_monsterDataFile));

    if(!areSameFile(_monsterUIFile, rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._monsterUI)))
        element.setAttribute("monsterUI", targetDirectory.relativeFilePath(_monsterUIFile));

    {
        const QString templateSpellbook = rulesetTemplate._spellbook.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellbook);
        if(!areSameFile(_spellbookFile, templateSpellbook))
            element.setAttribute("spellbook", targetDirectory.relativeFilePath(_spellbookFile));
    }

    {
        const QString templateSpellData = rulesetTemplate._spellData.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellData);
        if(!areSameFile(_spellDataFile, templateSpellData))
            element.setAttribute("spelldata", targetDirectory.relativeFilePath(_spellDataFile));
    }

    {
        const QString templateSpellUI = rulesetTemplate._spellUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._spellUI);
        if(!areSameFile(_spellUIFile, templateSpellUI))
            element.setAttribute("spellui", targetDirectory.relativeFilePath(_spellUIFile));
    }

    {
        const QString templateCombatantUI = rulesetTemplate._combatantUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantUI);
        if(!areSameFile(_combatantUIFile, templateCombatantUI))
            element.setAttribute("combatantui", targetDirectory.relativeFilePath(_combatantUIFile));
    }

    {
        const QString templateCombatantFrameUI = rulesetTemplate._combatantFrameUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantFrameUI);
        if(!areSameFile(_combatantFrameUIFile, templateCombatantFrameUI))
            element.setAttribute("combatantframeui", targetDirectory.relativeFilePath(_combatantFrameUIFile));
    }

    {
        const QString templateCombatantFrameData = rulesetTemplate._combatantFrameData.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._combatantFrameData);
        if(!areSameFile(_combatantFrameDataFile, templateCombatantFrameData))
            element.setAttribute("combatantframedata", targetDirectory.relativeFilePath(_combatantFrameDataFile));
    }

    {
        const QString templateLairActionUI = rulesetTemplate._lairActionUI.isEmpty() ? QString() : rulesetTemplate._rulesetDir.absoluteFilePath(rulesetTemplate._lairActionUI);
        if(!areSameFile(_lairActionUIFile, templateLairActionUI))
            element.setAttribute("lairactionui", targetDirectory.relativeFilePath(_lairActionUIFile));
    }

    if(_combatantDoneCheckbox != rulesetTemplate._combatantDone)
        element.setAttribute("combatantDone", _combatantDoneCheckbox);

    if(_hitPointsCountDown != rulesetTemplate._hitPointsCountDown)
        element.setAttribute("hitPointsCountDown", _hitPointsCountDown);

    if(_movementType != DMHelper::MovementType_Distance)
        element.setAttribute("movementType", movementStringFromType(_movementType, &_movementRanges));

    // Output condition deltas (only differences from ruleset defaults)
    if(_conditions && _rulesetDefaultConditions && _conditions->hasDeltasFrom(*_rulesetDefaultConditions))
        _conditions->outputDeltas(doc, element, *_rulesetDefaultConditions, targetDirectory);
}

bool Ruleset::areSameFile(const QString &file1, const QString &file2) const
{
    QFileInfo fileInfo1(file1);
    QFileInfo fileInfo2(file2);

    QString canonicalPath1 = fileInfo1.canonicalFilePath();
    QString canonicalPath2 = fileInfo2.canonicalFilePath();

    return ((!canonicalPath1.isEmpty()) && (canonicalPath1 == canonicalPath2));
}

void Ruleset::registerChange()
{
    if(_batchProcessing)
        _changed = true;
    else
        emit rulesetChanged();
}
