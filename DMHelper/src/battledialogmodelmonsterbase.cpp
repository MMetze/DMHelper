#include "battledialogmodelmonsterbase.h"
#include "monsterclassv2.h"
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
