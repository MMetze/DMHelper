#include "battledialogmodelmonsterbase.h"
//#include "monsterclass.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase() :
    BattleDialogModelCombatant(),
    _isShown(true),
    _isKnown(true)//,
    //_legendaryCount(-1)
{
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(int initiative, const QPointF& position) :
    BattleDialogModelCombatant(initiative, position),
    _isShown(true),
    _isKnown(true)//,
    //_legendaryCount(-1)
{
}

BattleDialogModelMonsterBase::BattleDialogModelMonsterBase(const BattleDialogModelMonsterBase& other) :
    BattleDialogModelCombatant(other),
    _isShown(other._isShown),
    _isKnown(other._isKnown)//,
    //_legendaryCount(other._legendaryCount)
{
}

BattleDialogModelMonsterBase::~BattleDialogModelMonsterBase()
{
}

void BattleDialogModelMonsterBase::inputXML(const QDomElement &element)
{
    BattleDialogModelCombatant::inputXML(element);

    _isShown = (bool)element.attribute("isShown",QString::number(0)).toInt();
    _isKnown = (bool)element.attribute("isKnown",QString::number(0)).toInt();
    //_legendaryCount = element.attribute("legendaryCount",QString::number(-1)).toInt();
}

int BattleDialogModelMonsterBase::getType() const
{
    return DMHelper::CombatantType_Monster;
}

BattleDialogModelCombatant* BattleDialogModelMonsterBase::clone() const
{
    return new BattleDialogModelMonsterBase(*this);
}


bool BattleDialogModelMonsterBase::getShown() const
{
    return _isShown;
}

bool BattleDialogModelMonsterBase::getKnown() const
{
    return _isKnown;
}

/*
int BattleDialogModelMonsterBase::getSkillModifier(Combatant::Skills skill) const
{
    MonsterClass* monsterClass = getMonsterClass();
    if(monsterClass)
        return monsterClass->getSkillValue(skill);
    else
        return 0;
}

int BattleDialogModelMonsterBase::getLegendaryCount() const
{
    return _legendaryCount;
}
*/

void BattleDialogModelMonsterBase::setShown(bool isShown)
{
    _isShown = isShown;
}

void BattleDialogModelMonsterBase::setKnown(bool isKnown)
{
    _isKnown = isKnown;
}

/*
void BattleDialogModelMonsterBase::setLegendaryCount(int legendaryCount)
{
    _legendaryCount = legendaryCount;
}
*/

int BattleDialogModelMonsterBase::getSizeFactor() const
{
    // TODO: get this
    return 1;
}

int BattleDialogModelMonsterBase::getSizeCategory() const
{
    // TODO: get this
    return DMHelper::CombatantSize_Medium;
}

QString BattleDialogModelMonsterBase::getName() const
{
    // TODO: get this
    return QString();
}


/*
void BattleDialogModelMonsterBase::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);

    element.setAttribute("monsterType", getMonsterType());
    element.setAttribute("isShown", _isShown);
    element.setAttribute("isKnown", _isKnown);
    element.setAttribute("legendaryCount", _legendaryCount);
}
*/

/*
int BattleDialogModelMonsterBase::convertSizeToFactor(const QString& monsterSize) const
{
    if(monsterSize == QString("Tiny"))
        return 1;
    else if(monsterSize == QString("Small"))
        return 1;
    else if(monsterSize == QString("Medium"))
        return 1;
    else if(monsterSize == QString("Large"))
        return 2;
    else if(monsterSize == QString("Huge"))
        return 3;
    else if(monsterSize == QString("Gargantuan"))
        return 4;
    else
        return 1;
}
*/
