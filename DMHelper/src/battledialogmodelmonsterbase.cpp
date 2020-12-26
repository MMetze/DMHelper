#include "battledialogmodelmonsterbase.h"
#include "monsterclass.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(const QString& name, QObject *parent) :
    BattleDialogModelCombatant(name, parent),
    _legendaryCount(-1),
    _conditions(0)
{
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant) :
    BattleDialogModelCombatant(combatant),
    _legendaryCount(-1),
    _conditions(0)
{
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(Combatant* combatant, int initiative, const QPointF& position) :
    BattleDialogModelCombatant(combatant, initiative, position),
    _legendaryCount(-1),
    _conditions(0)
{
}

BattleDialogModelMonsterBase::~BattleDialogModelMonsterBase()
{
}

void BattleDialogModelMonsterBase::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelCombatant::inputXML(element, isImport);

    _legendaryCount = element.attribute("legendaryCount",QString::number(-1)).toInt();
    _conditions = element.attribute("conditions",QString::number(0)).toInt();
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

int BattleDialogModelMonsterBase::getLegendaryCount() const
{
    return _legendaryCount;
}

void BattleDialogModelMonsterBase::setConditions(int conditions)
{
    _conditions = conditions;
}

void BattleDialogModelMonsterBase::applyConditions(int conditions)
{
    _conditions |= conditions;
}

void BattleDialogModelMonsterBase::setLegendaryCount(int legendaryCount)
{
    _legendaryCount = legendaryCount;
}

void BattleDialogModelMonsterBase::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("monsterType", getMonsterType());
    element.setAttribute("legendaryCount", _legendaryCount);
    element.setAttribute("conditions", _conditions);

    BattleDialogModelCombatant::internalOutputXML(doc, element, targetDirectory, isExport);
}
