#include "battledialogmodelmonsterbase.h"
#include "monsterclass.h"
#include <QDomElement>

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(const QString& name, QObject *parent) :
    BattleDialogModelCombatant(name, parent),
    _legendaryCount(-1),
    _conditions(0)
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant) :
    BattleDialogModelCombatant(combatant),
    _legendaryCount(-1),
    _conditions(0)
{
    connect(this, &BattleDialogModelMonsterBase::dataChanged, this, &BattleDialogModelMonsterBase::dirty);
    connect(this, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModelMonsterBase::dirty);
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant, int initiative, const QPointF& position) :
    BattleDialogModelCombatant(combatant, initiative, position),
    _legendaryCount(-1),
    _conditions(0)
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
    _conditions = element.attribute("conditions", QString::number(0)).toInt();
}

void BattleDialogModelMonsterBase::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelMonsterBase* otherMonsterBase = dynamic_cast<const BattleDialogModelMonsterBase*>(other);
    if(!otherMonsterBase)
        return;

    _legendaryCount = otherMonsterBase->_legendaryCount;
    _conditions = otherMonsterBase->_conditions;

    BattleDialogModelCombatant::copyValues(other);
}

int BattleDialogModelMonsterBase::getCombatantType() const
{
    return DMHelper::CombatantType_Monster;
}

int BattleDialogModelMonsterBase::getSkillModifier(Combatant::Skills skill) const
{
    MonsterClass* monsterClass = getMonsterClass();
    if(monsterClass)
        return monsterClass->getSkillValue(skill);
    else
        return 0;
}

int BattleDialogModelMonsterBase::getConditions() const
{
    return _conditions;
}

bool BattleDialogModelMonsterBase::hasCondition(Combatant::Condition condition) const
{
    return ((_conditions & condition) != 0);
}

int BattleDialogModelMonsterBase::getLegendaryCount() const
{
    return _legendaryCount;
}

void BattleDialogModelMonsterBase::setConditions(int conditions)
{
    if(_conditions != conditions)
    {
        _conditions = conditions;
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::applyConditions(int conditions)
{
    if((_conditions & conditions) != conditions)
    {
        _conditions |= conditions;
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::removeConditions(int conditions)
{
    if((_conditions & conditions) != 0)
    {
        _conditions &= ~conditions;
        emit dataChanged(this);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelMonsterBase::clearConditions()
{
    if(_conditions != 0)
    {
        _conditions = 0;
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

void BattleDialogModelMonsterBase::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("monsterType", getMonsterType());
    element.setAttribute("legendaryCount", _legendaryCount);
    element.setAttribute("conditions", _conditions);

    BattleDialogModelCombatant::internalOutputXML(doc, element, targetDirectory, isExport);
}
