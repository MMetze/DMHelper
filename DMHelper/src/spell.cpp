#include "spell.h"
#include "spellbook.h"
#include "battledialogmodeleffect.h"

Spell::Spell(const QString& name, QObject *parent) :
    QObject(parent),
    _name(name),
    _level(0),
    _school(),
    _time(),
    _range(),
    _components(),
    _duration(),
    _classes(),
    _description(),
    _ritual(false),
    _rolls(),
    _effectType(BattleDialogModelEffect::BattleDialogModelEffect_Base),
    _effectShapeActive(true),
    _effectSize(20, 20),
    _effectColor(115,18,0,64),
    _effectToken(),
    _effectTokenRotation(0),
    _effectConditions(Combatant::Condition_None),
    _batchChanges(false),
    _changesMade(false)
{
}

Spell::Spell(const QDomElement &element, bool isImport, QObject *parent) :
    QObject(parent),
    _name(),
    _level(0),
    _school(),
    _time(),
    _range(),
    _components(),
    _duration(),
    _classes(),
    _description(),
    _ritual(false),
    _rolls(),
    _effectType(BattleDialogModelEffect::BattleDialogModelEffect_Base),
    _effectShapeActive(true),
    _effectSize(20, 20),
    _effectColor(115,18,0,64),
    _effectToken(),
    _effectTokenRotation(0),
    _effectConditions(Combatant::Condition_None),
    _batchChanges(false),
    _changesMade(false)
{
    inputXML(element, isImport);
}

void Spell::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    beginBatchChanges();

    setName(element.firstChildElement(QString("name")).text());
    setLevel(element.firstChildElement(QString("level")).text().toInt());
    setSchool(element.firstChildElement(QString("school")).text());
    setTime(element.firstChildElement(QString("time")).text());
    setRange(element.firstChildElement(QString("range")).text());
    setComponents(element.firstChildElement(QString("components")).text());
    setDuration(element.firstChildElement(QString("duration")).text());
    setClasses(element.firstChildElement(QString("classes")).text());

    QString ritualText = element.firstChildElement(QString("ritual")).text();
    setRitual((!ritualText.isEmpty()) && (ritualText != QString("NO")) && (ritualText != QString("0")));

    QString descriptionText;
    QDomElement descriptionElement = element.firstChildElement(QString("text"));
    while(!descriptionElement.isNull())
    {
        descriptionText.append(descriptionElement.text());
        descriptionText.append(QChar::LineFeed);
        descriptionText.append(QChar::LineFeed);
        descriptionElement = descriptionElement.nextSiblingElement(QString("text"));
    }
    setDescription(descriptionText);

    QDomElement rollElement = element.firstChildElement(QString("roll"));
    while(!rollElement.isNull())
    {
        addRoll(rollElement.text());
        rollElement = rollElement.nextSiblingElement(QString("roll"));
    }

    QDomElement effectElement = element.firstChildElement(QString("effect"));
    if(!effectElement.isNull())
    {
        setEffectType(effectElement.attribute("type", QString::number(0)).toInt());
        setEffectShapeActive(static_cast<bool>(element.attribute("shapeActive", QString::number(1)).toInt()));
        setEffectSize(QSize(effectElement.attribute("sizeX", QString::number(20)).toInt(),
                            effectElement.attribute("sizeY", QString::number(20)).toInt()));
        setEffectColor(QColor(effectElement.attribute("colorR", QString::number(115)).toInt(),
                              effectElement.attribute("colorG", QString::number(18)).toInt(),
                              effectElement.attribute("colorB", QString::number(0)).toInt(),
                              effectElement.attribute("colorA", QString::number(64)).toInt()));
        setEffectConditions(effectElement.attribute("conditions", QString("0")).toInt());
        setEffectToken(effectElement.firstChildElement(QString("token")).text());
        setEffectTokenRotation(effectElement.attribute("tokenRotation", QString("0")).toInt());
    }

    endBatchChanges();
}

void Spell::inputXML_CONVERT(const QDomElement &element)
{
    beginBatchChanges();

    setName(element.firstChildElement(QString("name")).text());
    setLevel(element.firstChildElement(QString("level")).text().toInt());
    QString schoolText = element.firstChildElement(QString("school")).text();
    schoolText.replace(0, 1, schoolText.at(0).toUpper());
    setSchool(schoolText);
    setTime(element.firstChildElement(QString("casting_time")).text());
    setRange(element.firstChildElement(QString("range")).text());
    setDuration(element.firstChildElement(QString("duration")).text());

    QString ritualText = element.firstChildElement(QString("ritual")).text();
    setRitual(ritualText == QString("true"));

    setDescription(element.firstChildElement(QString("description")).text());

    QString classesString;
    QDomElement classesElement = element.firstChildElement(QString("classes"));
    QDomElement classElement = classesElement.firstChildElement(QString("element"));
    while(!classElement.isNull())
    {
        if(!classesString.isEmpty())
            classesString += QString(", ");

        QString classString = classElement.text();
        classString.replace(0, 1, classString.at(0).toUpper());
        classesString += classString;

        classElement = classElement.nextSiblingElement(QString("element"));
    }
    setClasses(classesString);

    QDomElement componentsElement = element.firstChildElement(QString("components"));
    if(!componentsElement.isNull())
        setComponents(componentsElement.firstChildElement(QString("raw")).text());

    endBatchChanges();
}

QDomElement Spell::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(targetDirectory);

    outputValue(doc, element, isExport, QString("name"), getName());
    outputValue(doc, element, isExport, QString("level"), QString::number(getLevel()));
    outputValue(doc, element, isExport, QString("school"), getSchool());
    outputValue(doc, element, isExport, QString("time"), getTime());
    outputValue(doc, element, isExport, QString("range"), getRange());
    outputValue(doc, element, isExport, QString("components"), getComponents());
    outputValue(doc, element, isExport, QString("duration"), getDuration());
    outputValue(doc, element, isExport, QString("classes"), getClasses());

    if(isRitual())
        outputValue(doc, element, isExport, QString("ritual"), QString("YES"));

    QStringList descriptionParts = getDescription().split(QChar::LineFeed);
    for(QString& part : descriptionParts)
    {
        if(!part.isEmpty())
            outputValue(doc, element, isExport, QString("text"), part);
    }

    for(Dice& singleRoll : getRolls())
    {
        outputValue(doc, element, isExport, QString("roll"), singleRoll.toString());
    }

    QDomElement effectElement = doc.createElement(QString("effect"));
    effectElement.setAttribute("type", getEffectType());
    effectElement.setAttribute("shapeActive", getEffectShapeActive());
    effectElement.setAttribute("sizeX", getEffectSize().width());
    effectElement.setAttribute("sizeY", getEffectSize().height());
    effectElement.setAttribute("colorR", getEffectColor().red());
    effectElement.setAttribute("colorG", getEffectColor().green());
    effectElement.setAttribute("colorB", getEffectColor().blue());
    effectElement.setAttribute("colorA", getEffectColor().alpha());
    effectElement.setAttribute("conditions", getEffectConditions());
    effectElement.setAttribute("tokenRotation", getEffectTokenRotation());
    //outputValue(doc, effectElement, isExport, QString("token"), getEffectToken().isEmpty() ? QString("") : getEffectToken());
    outputValue(doc, effectElement, isExport, QString("token"), getEffectToken().isEmpty() ? QString("") : targetDirectory.relativeFilePath(getEffectToken()));
    element.appendChild(effectElement);

    return element;
}

void Spell::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
}

void Spell::endBatchChanges()
{
    if(_batchChanges)
    {
        _batchChanges = false;
        if(_changesMade)
        {
            emit dirty();
        }
    }
}

void Spell::cloneSpell(Spell& other)
{
    beginBatchChanges();

    _name = other._name;
    _level = other._level;
    _school = other._school;
    _time = other._time;
    _range = other._range;
    _components = other._components;
    _duration = other._duration;
    _classes = other._classes;
    _description = other._description;
    _ritual = other._ritual;

    _rolls.clear(); // just in case we're cloning onto something that exists
    for(Dice roll : other._rolls)
        _rolls.append(roll);

    _effectType = other._effectType;
    _effectShapeActive = other._effectShapeActive;
    _effectSize = other._effectSize;
    _effectColor = other._effectColor;
    _effectToken = other._effectToken;
    _effectTokenRotation = other._effectTokenRotation;
    _effectConditions = other._effectConditions;

    endBatchChanges();
}

void Spell::outputValue(QDomDocument &doc, QDomElement &element, bool isExport, const QString& valueName, const QString& valueText)
{
    Q_UNUSED(isExport);

    QDomElement newChild = doc.createElement(valueName);
    newChild.appendChild(doc.createTextNode(valueText));
    element.appendChild(newChild);
}

QString Spell::getName() const
{
    return _name;
}

int Spell::getLevel() const
{
    return _level;
}

QString Spell::getSchool() const
{
    return _school;
}

QString Spell::getTime() const
{
    return _time;
}

QString Spell::getRange() const
{
    return _range;
}

QString Spell::getComponents() const
{
    return _components;
}

QString Spell::getDuration() const
{
    return _duration;
}

QString Spell::getClasses() const
{
    return _classes;
}

QString Spell::getDescription() const
{
    return _description;
}

bool Spell::isRitual() const
{
    return _ritual;
}

QList<Dice> Spell::getRolls() const
{
    return _rolls;
}

QString Spell::getRollsString() const
{
    QString result;

    for(Dice singleRoll : getRolls())
    {
        result += singleRoll.toString() + QChar::LineFeed;
    }

    return result;
}

int Spell::getEffectType() const
{
    return _effectType;
}

bool Spell::getEffectShapeActive() const
{
    return _effectShapeActive;
}

QSize Spell::getEffectSize() const
{
    return _effectSize;
}

QColor Spell::getEffectColor() const
{
    return _effectColor;
}
QString Spell::getEffectToken() const
{
    return _effectToken;
}

QString Spell::getEffectTokenPath() const
{
    return Spellbook::Instance()->getDirectory().filePath(getEffectToken());
}

int Spell::getEffectTokenRotation() const
{
    return _effectTokenRotation;
}

int Spell::getEffectConditions() const
{
    return _effectConditions;
}

bool Spell::hasEffectCondition(Combatant::Condition condition) const
{
    return ((_effectConditions & condition) != 0);
}

void Spell::setName(const QString& name)
{
    if(name == _name)
        return;

    _name = name;
    registerChange();
}

void Spell::setLevel(int level)
{
    if(level == _level)
        return;

    _level = level;
    registerChange();
}

void Spell::setSchool(const QString& school)
{
    if(school == _school)
        return;

    _school = school;
    registerChange();
}

void Spell::setTime(const QString& time)
{
    if(time == _time)
        return;

    _time = time;
    registerChange();
}

void Spell::setRange(const QString& range)
{
    if(range == _range)
        return;

    _range = range;
    registerChange();
}

void Spell::setComponents(const QString& components)
{
    if(components == _components)
        return;

    _components = components;
    registerChange();
}

void Spell::setDuration(const QString& duration)
{
    if(duration == _duration)
        return;

    _duration = duration;
    registerChange();
}

void Spell::setClasses(const QString& classes)
{
    if(classes == _classes)
        return;

    _classes = classes;
    registerChange();
}

void Spell::setDescription(const QString& description)
{
    if(description == _description)
        return;

    _description = description;
    registerChange();
}

void Spell::setRitual(bool ritual)
{
    if(ritual == _ritual)
        return;

    _ritual = ritual;
    registerChange();
}

void Spell::setRolls(const QList<Dice>& rolls)
{
    if(rolls == _rolls)
        return;

    _rolls = rolls;
    registerChange();
}

void Spell::addRoll(const Dice& roll)
{
    _rolls.append(roll);
    registerChange();
}

void Spell::addRoll(const QString& roll)
{
    Dice newDice(roll);
    if(newDice.getType() > 0)
        addRoll(newDice);
}

void Spell::setEffectType(int effectType)
{
    if(_effectType == effectType)
        return;

    _effectType = effectType;
    registerChange();
}

void Spell::setEffectShapeActive(bool effectShapeActive)
{
    if(_effectShapeActive == effectShapeActive)
        return;

    _effectShapeActive = effectShapeActive;
    registerChange();
}

void Spell::setEffectSize(QSize effectSize)
{
    if(_effectSize == effectSize)
        return;

    _effectSize = effectSize;
    registerChange();
}

void Spell::setEffectColor(QColor effectColor)
{
    if(_effectColor == effectColor)
        return;

    _effectColor = effectColor;
    registerChange();
}

void Spell::setEffectToken(QString effectToken)
{
    if(_effectToken == effectToken)
        return;

    _effectToken = effectToken;
    registerChange();
}

void Spell::setEffectTokenRotation(int effectTokenRotation)
{
    if(_effectTokenRotation == effectTokenRotation)
        return;

    _effectTokenRotation = effectTokenRotation;
    registerChange();
}

void Spell::setEffectConditions(int conditions)
{
    if(_effectConditions == conditions)
        return;

    _effectConditions = conditions;
    registerChange();
}

void Spell::applyEffectConditions(int conditions)
{
    if((_effectConditions & conditions) != conditions)
    {
        _effectConditions |= conditions;
        registerChange();
    }
}

void Spell::addEffectCondition(Combatant::Condition condition)
{
    if(!hasEffectCondition(condition))
    {
        _effectConditions |= condition;
        registerChange();
    }
}

void Spell::removeEffectCondition(Combatant::Condition condition)
{
    if(hasEffectCondition(condition))
    {
        _effectConditions &= ~condition;
        registerChange();
    }
}

void Spell::clearRolls()
{
    _rolls.clear();
    registerChange();
}

void Spell::registerChange()
{
    if(_batchChanges)
        _changesMade = true;
    else
        emit dirty();
}
