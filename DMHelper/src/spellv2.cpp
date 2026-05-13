#include "spellv2.h"
#include "spellbookfactory.h"
#include "spellbook.h"
#include "battledialogmodeleffect.h"
#include <QDomElement>
#include <QDomDocument>
#include <QDir>
#include <QStringList>

namespace
{
    // Default colour for a freshly-minted spell effect (matches legacy Spell).
    static const int kDefaultEffectColorR = 115;
    static const int kDefaultEffectColorG = 18;
    static const int kDefaultEffectColorB = 0;
    static const int kDefaultEffectColorA = 64;
    static const int kDefaultEffectSize = 20;
}

Spellv2::Spellv2(const QString& name, QObject *parent) :
    QObject{parent},
    TemplateObject{SpellbookFactory::Instance()},
    _allValues(),
    _batchChanges(false),
    _changesMade(false)
{
    setStringValue("name", name);
}

Spellv2::Spellv2(const QDomElement &element, bool isImport, QObject *parent) :
    QObject{parent},
    TemplateObject{SpellbookFactory::Instance()},
    _allValues(),
    _batchChanges(false),
    _changesMade(false)
{
    inputXML(element, isImport);
}

void Spellv2::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();
    readXMLValues(element, isImport);
    endBatchChanges();
}

QDomElement Spellv2::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    writeXMLValues(doc, element, targetDirectory, isExport);
    return element;
}

void Spellv2::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
}

void Spellv2::endBatchChanges()
{
    if(_batchChanges)
    {
        _batchChanges = false;
        if(_changesMade)
            emit dirty();
    }
}

void Spellv2::cloneSpell(Spellv2& other)
{
    beginBatchChanges();
    copyValues(other);
    endBatchChanges();
}

QHash<QString, QVariant>* Spellv2::valueHash()
{
    return &_allValues;
}

const QHash<QString, QVariant>* Spellv2::valueHash() const
{
    return &_allValues;
}

void Spellv2::declareDirty()
{
    registerChange();
}

void Spellv2::registerChange()
{
    if(_batchChanges)
        _changesMade = true;
    else
        emit dirty();
}

// ---------------------------------------------------------------------------
// Getters (all read through TemplateObject's value-bag)
// ---------------------------------------------------------------------------

QString Spellv2::getName() const            { return getStringValue("name"); }
int Spellv2::getLevel() const               { return getIntValue("level"); }
QString Spellv2::getSchool() const          { return getStringValue("school"); }
QString Spellv2::getTime() const            { return getStringValue("time"); }
QString Spellv2::getRange() const           { return getStringValue("range"); }
QString Spellv2::getComponents() const      { return getStringValue("components"); }
QString Spellv2::getDuration() const        { return getStringValue("duration"); }
QString Spellv2::getClasses() const         { return getStringValue("classes"); }
QString Spellv2::getDescription() const     { return getStringValue("description"); }
bool Spellv2::isRitual() const              { return getBoolValue("ritual"); }

int Spellv2::getEffectType() const
{
    if(_allValues.contains("effect_type"))
        return getIntValue("effect_type");
    return BattleDialogModelEffect::BattleDialogModelEffect_Base;
}

bool Spellv2::getEffectShapeActive() const
{
    if(_allValues.contains("effect_shape_active"))
        return getBoolValue("effect_shape_active");
    return true;
}

QSize Spellv2::getEffectSize() const
{
    const int w = _allValues.contains("effect_size_w") ? getIntValue("effect_size_w") : kDefaultEffectSize;
    const int h = _allValues.contains("effect_size_h") ? getIntValue("effect_size_h") : kDefaultEffectSize;
    return QSize(w, h);
}

QColor Spellv2::getEffectColor() const
{
    if(_allValues.contains("effect_color"))
    {
        const QString hex = getStringValue("effect_color");
        QColor c(hex);
        if(c.isValid())
            return c;
    }
    return QColor(kDefaultEffectColorR, kDefaultEffectColorG, kDefaultEffectColorB, kDefaultEffectColorA);
}

QString Spellv2::getEffectToken() const     { return getStringValue("effect_token"); }

QString Spellv2::getEffectTokenPath() const
{
    if((!Spellbook::Instance()) || (getEffectToken().isEmpty()))
        return QString();
    return Spellbook::Instance()->getDirectory().filePath(getEffectToken());
}

int Spellv2::getEffectTokenRotation() const { return getIntValue("effect_token_rotation"); }

QStringList Spellv2::getEffectConditionList() const
{
    const QString joined = getStringValue("effect_conditions");
    if(joined.isEmpty())
        return QStringList();
    return joined.split(QStringLiteral(","), Qt::SkipEmptyParts);
}

bool Spellv2::hasEffectCondition(const QString& conditionId) const
{
    return getEffectConditionList().contains(conditionId);
}

QList<Dice> Spellv2::getRolls() const
{
    QList<Dice> result;
    const QString joined = getStringValue("rolls");
    if(joined.isEmpty())
        return result;

    const QStringList parts = joined.split(QStringLiteral(","), Qt::SkipEmptyParts);
    for(const QString& part : parts)
        result.append(Dice(part.trimmed()));
    return result;
}

QString Spellv2::getRollsString() const
{
    QString result;
    const QList<Dice> all = getRolls();
    for(const Dice& d : all)
        result += d.toString() + QChar::LineFeed;
    return result;
}

// ---------------------------------------------------------------------------
// Setters
// ---------------------------------------------------------------------------

void Spellv2::setName(const QString& name)               { setStringValue("name", name); }
void Spellv2::setLevel(int level)                        { setIntValue("level", level); }
void Spellv2::setSchool(const QString& school)           { setStringValue("school", school); }
void Spellv2::setTime(const QString& time)               { setStringValue("time", time); }
void Spellv2::setRange(const QString& range)             { setStringValue("range", range); }
void Spellv2::setComponents(const QString& components)   { setStringValue("components", components); }
void Spellv2::setDuration(const QString& duration)       { setStringValue("duration", duration); }
void Spellv2::setClasses(const QString& classes)         { setStringValue("classes", classes); }
void Spellv2::setDescription(const QString& description) { setStringValue("description", description); }
void Spellv2::setRitual(bool ritual)                     { setBoolValue("ritual", ritual); }

void Spellv2::setEffectType(int effectType)              { setIntValue("effect_type", effectType); }
void Spellv2::setEffectShapeActive(bool active)          { setBoolValue("effect_shape_active", active); }

void Spellv2::setEffectSize(QSize effectSize)
{
    beginBatchChanges();
    setIntValue("effect_size_w", effectSize.width());
    setIntValue("effect_size_h", effectSize.height());
    endBatchChanges();
}

void Spellv2::setEffectColor(const QColor& effectColor)
{
    setStringValue("effect_color", effectColor.name(QColor::HexArgb));
}

void Spellv2::setEffectToken(QString effectToken)        { setStringValue("effect_token", effectToken); }
void Spellv2::setEffectTokenRotation(int rotation)       { setIntValue("effect_token_rotation", rotation); }

void Spellv2::setEffectConditionList(const QStringList& conditions)
{
    setStringValue("effect_conditions", conditions.join(QStringLiteral(",")));
}

void Spellv2::addEffectCondition(const QString& conditionId)
{
    QStringList list = getEffectConditionList();
    if(list.contains(conditionId))
        return;
    list.append(conditionId);
    setEffectConditionList(list);
}

void Spellv2::removeEffectCondition(const QString& conditionId)
{
    QStringList list = getEffectConditionList();
    if(!list.removeOne(conditionId))
        return;
    setEffectConditionList(list);
}

void Spellv2::setRolls(const QList<Dice>& rolls)
{
    QStringList parts;
    parts.reserve(rolls.size());
    for(const Dice& d : rolls)
        parts.append(d.toString());
    setStringValue("rolls", parts.join(QStringLiteral(",")));
}

void Spellv2::addRoll(const Dice& roll)
{
    QList<Dice> rolls = getRolls();
    rolls.append(roll);
    setRolls(rolls);
}

void Spellv2::addRoll(const QString& roll)
{
    addRoll(Dice(roll));
}

void Spellv2::clearRolls()
{
    setStringValue("rolls", QString());
}
