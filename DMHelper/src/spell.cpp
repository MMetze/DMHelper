#include "spell.h"

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
        descriptionElement = descriptionElement.nextSiblingElement(QString("text"));
    }
    setDescription(descriptionText);

    QDomElement rollElement = element.firstChildElement(QString("roll"));
    while(!rollElement.isNull())
    {
        //addRoll(Dice(rollElement.text()));
        addRoll(rollElement.text());
        rollElement = rollElement.nextSiblingElement(QString("roll"));
    }

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
    for(QString part : descriptionParts)
    {
        outputValue(doc, element, isExport, QString("text"), part);
    }

    for(QString singleRoll : getRolls())
    {
        outputValue(doc, element, isExport, QString("roll"), singleRoll);
    }

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
    //for(Dice roll : other._rolls)
    for(QString roll : other._rolls)
        _rolls.append(roll);

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

//QList<Dice> Spell::getRolls() const
QStringList Spell::getRolls() const
{
    return _rolls;
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

//void Spell::setRolls(const QList<Dice>& rolls)
void Spell::setRolls(const QStringList& rolls)
{
    if(rolls == _rolls)
        return;

    _rolls = rolls;
    registerChange();
}

//void Spell::addRoll(const Dice& roll)
void Spell::addRoll(const QString& roll)
{
    _rolls.append(roll);
    registerChange();
}

void Spell::clearRolls()
{
    _rolls.clear();
    registerChange();
}

void Spell::registerChange()
{
    if(_batchChanges)
    {
        _changesMade = true;
    }
    else
    {
        emit dirty();
    }
}

