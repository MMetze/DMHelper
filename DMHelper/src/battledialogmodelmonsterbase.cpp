#include "battledialogmodelmonsterbase.h"
#include "conditions.h"
#include "monsterclassv2.h"
#include "perroundresource.h"
#include <QDomDocument>
#include <QDomElement>

static const char* RESOURCE_COUNTER_ELEMENT = "resourceCounter";
static const char* RESOURCE_COUNTER_ATTR_NAME = "name";
static const char* RESOURCE_COUNTER_ATTR_CURRENT = "current";
static const char* LEGACY_LEGENDARY_RESOURCE_NAME = "Legendary Actions";

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(const QString& name, QObject *parent) :
    BattleDialogModelCombatant(name, parent),
    _legendaryCount(-1),
    _monsterMaxHP(-1),
    _conditionList(),
    _resourceCounters()
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant) :
    BattleDialogModelCombatant(combatant),
    _legendaryCount(-1),
    _monsterMaxHP(-1),
    _conditionList(),
    _resourceCounters()
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant, int initiative, const QPointF& position) :
    BattleDialogModelCombatant(combatant, initiative, position),
    _legendaryCount(-1),
    _monsterMaxHP(-1),
    _conditionList(),
    _resourceCounters()
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::~BattleDialogModelMonsterBase()
{
}

void BattleDialogModelMonsterBase::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelCombatant::inputXML(element, isImport);

    _legendaryCount = element.attribute("legendaryCount", QString::number(-1)).toInt();
    _monsterMaxHP = element.attribute("monsterMaxHP", QString::number(-1)).toInt();

    // Read per-instance resource counters from <resourceCounter> child elements.
    _resourceCounters.clear();
    QDomElement counterElement = element.firstChildElement(QString::fromLatin1(RESOURCE_COUNTER_ELEMENT));
    while(!counterElement.isNull())
    {
        const QString name = counterElement.attribute(QString::fromLatin1(RESOURCE_COUNTER_ATTR_NAME));
        if(!name.isEmpty())
        {
            const int current = counterElement.attribute(QString::fromLatin1(RESOURCE_COUNTER_ATTR_CURRENT), QStringLiteral("0")).toInt();
            _resourceCounters.insert(name, current);
        }
        counterElement = counterElement.nextSiblingElement(QString::fromLatin1(RESOURCE_COUNTER_ELEMENT));
    }

    // Legacy migration: if the old legendaryCount attribute is set and no "Legendary Actions"
    // counter was loaded from the new format, seed the new counter from the legacy field.
    if((_legendaryCount > -1) && (!_resourceCounters.contains(QString::fromLatin1(LEGACY_LEGENDARY_RESOURCE_NAME))))
        _resourceCounters.insert(QString::fromLatin1(LEGACY_LEGENDARY_RESOURCE_NAME), _legendaryCount);

    // Condition migration: detect old int bitmask format vs new comma-separated string IDs
    QString condStr = element.attribute("conditions", QString());
    if(!condStr.isEmpty())
    {
        bool ok = false;
        int condInt = condStr.toInt(&ok);
        if(ok)
            _conditionList = Conditions::migrateFromBitmask(condInt);
        else
            _conditionList = condStr.split(QStringLiteral(","), Qt::SkipEmptyParts);
    }
}

void BattleDialogModelMonsterBase::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelMonsterBase* otherMonsterBase = dynamic_cast<const BattleDialogModelMonsterBase*>(other);
    if(!otherMonsterBase)
        return;

    _legendaryCount = otherMonsterBase->_legendaryCount;
    _monsterMaxHP = otherMonsterBase->_monsterMaxHP;
    _conditionList = otherMonsterBase->_conditionList;
    _resourceCounters = otherMonsterBase->_resourceCounters;

    BattleDialogModelCombatant::copyValues(other);
}

int BattleDialogModelMonsterBase::getCombatantType() const
{
    return DMHelper::CombatantType_Monster;
}

int BattleDialogModelMonsterBase::getSkillModifier(Combatant::Skills skill) const
{
    MonsterClassv2* monsterClass = getMonsterClass();
    if(!monsterClass)
        return 0;

    // TODO: HACK
    switch(skill)
    {
        case Combatant::Skills_strengthSave:
            return monsterClass->getIntValue(QString("strengthSave"));
        case Combatant::Skills_athletics:
            return monsterClass->getIntValue(QString("athletics"));
        case Combatant::Skills_dexteritySave:
            return monsterClass->getIntValue(QString("dexteritySave"));
        case Combatant::Skills_stealth:
            return monsterClass->getIntValue(QString("stealth"));
        case Combatant::Skills_acrobatics:
            return monsterClass->getIntValue(QString("acrobatics"));
        case Combatant::Skills_sleightOfHand:
            return monsterClass->getIntValue(QString("sleightOfHand"));
        case Combatant::Skills_constitutionSave:
            return monsterClass->getIntValue(QString("constitutionSave"));
        case Combatant::Skills_intelligenceSave:
            return monsterClass->getIntValue(QString("intelligenceSave"));
        case Combatant::Skills_investigation:
            return monsterClass->getIntValue(QString("investigation"));
        case Combatant::Skills_arcana:
            return monsterClass->getIntValue(QString("arcana"));
        case Combatant::Skills_nature:
            return monsterClass->getIntValue(QString("nature"));
        case Combatant::Skills_history:
            return monsterClass->getIntValue(QString("history"));
        case Combatant::Skills_religion:
            return monsterClass->getIntValue(QString("religion"));
        case Combatant::Skills_wisdomSave:
            return monsterClass->getIntValue(QString("wisdomSave"));
        case Combatant::Skills_medicine:
            return monsterClass->getIntValue(QString("medicine"));
        case Combatant::Skills_animalHandling:
            return monsterClass->getIntValue(QString("animalHandling"));
        case Combatant::Skills_perception:
            return monsterClass->getIntValue(QString("perception"));
        case Combatant::Skills_insight:
            return monsterClass->getIntValue(QString("insight"));
        case Combatant::Skills_survival:
            return monsterClass->getIntValue(QString("survival"));
        case Combatant::Skills_charismaSave:
            return monsterClass->getIntValue(QString("charismaSave"));
        case Combatant::Skills_performance:
            return monsterClass->getIntValue(QString("performance"));
        case Combatant::Skills_deception:
            return monsterClass->getIntValue(QString("deception"));
        case Combatant::Skills_persuasion:
            return monsterClass->getIntValue(QString("persuasion"));
        case Combatant::Skills_intimidation:
            return monsterClass->getIntValue(QString("intimidation"));
        default:
            return 0;
    }
}

QStringList BattleDialogModelMonsterBase::getConditionList() const
{
    return _conditionList;
}

bool BattleDialogModelMonsterBase::hasConditionId(const QString& conditionId) const
{
    return _conditionList.contains(conditionId);
}

int BattleDialogModelMonsterBase::getLegendaryCount() const
{
    return _legendaryCount;
}

int BattleDialogModelMonsterBase::getMonsterMaxHP() const
{
    return _monsterMaxHP;
}

void BattleDialogModelMonsterBase::setMonsterMaxHP(int monsterMaxHP)
{
    if(_monsterMaxHP != monsterMaxHP)
    {
        _monsterMaxHP = monsterMaxHP;
        emit dataChanged(this);
    }
}

void BattleDialogModelMonsterBase::setConditionList(const QStringList& conditions)
{
    if(_conditionList != conditions)
    {
        _conditionList = conditions;
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::addConditionId(const QString& conditionId)
{
    if(!conditionId.isEmpty() && !_conditionList.contains(conditionId))
    {
        _conditionList.append(conditionId);
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::removeConditionId(const QString& conditionId)
{
    if(_conditionList.removeOne(conditionId))
    {
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::clearConditions()
{
    if(!_conditionList.isEmpty())
    {
        _conditionList.clear();
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::setLegendaryCount(int legendaryCount)
{
    if(_legendaryCount != legendaryCount)
    {
        _legendaryCount = legendaryCount;
        emit dataChanged(this);
    }
}

int BattleDialogModelMonsterBase::getResourceCount(const QString& resourceName) const
{
    return _resourceCounters.value(resourceName, 0);
}

bool BattleDialogModelMonsterBase::hasResourceCounter(const QString& resourceName) const
{
    return _resourceCounters.contains(resourceName);
}

QStringList BattleDialogModelMonsterBase::getResourceCounterNames() const
{
    return _resourceCounters.keys();
}

void BattleDialogModelMonsterBase::setResourceCount(const QString& resourceName, int count)
{
    if(resourceName.isEmpty())
        return;

    if((_resourceCounters.contains(resourceName)) && (_resourceCounters.value(resourceName) == count))
        return;

    _resourceCounters.insert(resourceName, count);
    emit resourceCountChanged(this, resourceName, count);
    emit dataChanged(this);
}

void BattleDialogModelMonsterBase::resetResources(const QString& scope)
{
    MonsterClassv2* monsterClass = getMonsterClass();
    if(!monsterClass)
        return;

    const QList<PerRoundResource> definedResources = monsterClass->getPerRoundResources();
    for(const PerRoundResource& resource : definedResources)
    {
        if((!scope.isEmpty()) && (resource.recharge != scope))
            continue;

        setResourceCount(resource.name, resource.max);
    }
}

void BattleDialogModelMonsterBase::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("monsterType", getMonsterType());
    element.setAttribute("legendaryCount", _legendaryCount);
    if(_monsterMaxHP > 0)
        element.setAttribute("monsterMaxHP", _monsterMaxHP);
    element.setAttribute("conditions", _conditionList.join(QStringLiteral(",")));

    // Persist per-instance resource counters as <resourceCounter> child elements.
    for(auto it = _resourceCounters.constBegin(); it != _resourceCounters.constEnd(); ++it)
    {
        QDomElement counterElement = doc.createElement(QString::fromLatin1(RESOURCE_COUNTER_ELEMENT));
        counterElement.setAttribute(QString::fromLatin1(RESOURCE_COUNTER_ATTR_NAME), it.key());
        counterElement.setAttribute(QString::fromLatin1(RESOURCE_COUNTER_ATTR_CURRENT), it.value());
        element.appendChild(counterElement);
    }

    BattleDialogModelCombatant::internalOutputXML(doc, element, targetDirectory, isExport);
}
