#include "battledialogmodelmonstercombatant.h"
#include "monster.h"
#include "monsterclassv2.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelMonsterCombatant::BattleDialogModelMonsterCombatant(const QString& name, QObject *parent) :
    BattleDialogModelMonsterBase(name, parent),
    _monsterSize(DMHelper::CombatantSize_Unknown),
    _monsterName(),
    _monsterHP(-1)
{
}

BattleDialogModelMonsterCombatant::BattleDialogModelMonsterCombatant(Monster* monster) :
    BattleDialogModelMonsterBase(monster),
    _monsterSize(DMHelper::CombatantSize_Unknown),
    _monsterName(),
    _monsterHP(-1)
{
}

BattleDialogModelMonsterCombatant::BattleDialogModelMonsterCombatant(Monster* monster, const QString& monsterName, int monsterSize, int monsterHP) :
    BattleDialogModelMonsterBase(monster),
    _monsterSize(monsterSize),
    _monsterName(monsterName),
    _monsterHP(monsterHP)
{
}

BattleDialogModelMonsterCombatant::BattleDialogModelMonsterCombatant(Monster* monster, const QString& monsterName, int monsterSize, int monsterHP, int initiative, const QPointF& position) :
    BattleDialogModelMonsterBase(monster, initiative, position),
    _monsterSize(monsterSize),
    _monsterName(monsterName),
    _monsterHP(monsterHP)
{
}

BattleDialogModelMonsterCombatant::~BattleDialogModelMonsterCombatant()
{
}

void BattleDialogModelMonsterCombatant::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelMonsterBase::inputXML(element, isImport);

    _monsterSize = element.attribute("monsterSize", QString::number(DMHelper::CombatantSize_Medium)).toInt();
    _monsterName = element.attribute("monsterName");
    _monsterHP = element.attribute("monsterHP", QString::number(0)).toInt();
}

void BattleDialogModelMonsterCombatant::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelMonsterCombatant* otherMonsterCombatant = dynamic_cast<const BattleDialogModelMonsterCombatant*>(other);
    if(!otherMonsterCombatant)
        return;

    _monsterSize = otherMonsterCombatant->_monsterSize;
    _monsterName = otherMonsterCombatant->_monsterName;
    _monsterHP = otherMonsterCombatant->_monsterHP;

    BattleDialogModelMonsterBase::copyValues(other);
}

BattleDialogModelCombatant* BattleDialogModelMonsterCombatant::clone() const
{
    BattleDialogModelMonsterCombatant* newMonster = new BattleDialogModelMonsterCombatant(getName());

    newMonster->copyValues(this);

    return newMonster;
}

qreal BattleDialogModelMonsterCombatant::getSizeFactor() const
{
    return MonsterClass::convertSizeCategoryToScaleFactor(getSizeCategory());
}

int BattleDialogModelMonsterCombatant::getSizeCategory() const
{
    if(_monsterSize > DMHelper::CombatantSize_Unknown)
        return _monsterSize;

    MonsterClassv2* monsterClass = getMonsterClass();
    if(!monsterClass)
        return DMHelper::CombatantSize_Unknown;

    return monsterClass->getMonsterSizeCategory();
}

int BattleDialogModelMonsterCombatant::getStrength() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getStrength();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getStrength!";
        return 0;
    }
}

int BattleDialogModelMonsterCombatant::getDexterity() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getDexterity();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getDexterity!";
        return 0;
    }
}

int BattleDialogModelMonsterCombatant::getConstitution() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getConstitution();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getConstitution!";
        return 0;
    }
}

int BattleDialogModelMonsterCombatant::getIntelligence() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getIntelligence();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getIntelligence!";
        return 0;
    }
}

int BattleDialogModelMonsterCombatant::getWisdom() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getWisdom();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getWisdom!";
        return 0;
    }
}

int BattleDialogModelMonsterCombatant::getCharisma() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getCharisma();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getCharisma!";
        return 0;
    }
}

int BattleDialogModelMonsterCombatant::getSpeed() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getSpeed();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getSpeed!";
        return 30;
    }
}

int BattleDialogModelMonsterCombatant::getArmorClass() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getArmorClass();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getArmorClass!";
        return 10;
    }
}

int BattleDialogModelMonsterCombatant::getHitPoints() const
{
    return _monsterHP;
}

void BattleDialogModelMonsterCombatant::setHitPoints(int hitPoints)
{
    if(_monsterHP != hitPoints)
    {
        _monsterHP = hitPoints;
        emit dataChanged(this);
    }
}

QString BattleDialogModelMonsterCombatant::getName() const
{
    return _monsterName;
}

QPixmap BattleDialogModelMonsterCombatant::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    if(_combatant)
    {
        return _combatant->getIconPixmap(iconSize);
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getIconPixmap!";
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
    }
}

int BattleDialogModelMonsterCombatant::getMonsterType() const
{
    return BattleMonsterType_Combatant;
}

MonsterClassv2* BattleDialogModelMonsterCombatant::getMonsterClass() const
{
    Monster* monster = getMonster();
    if(monster)
    {
        return monster->getMonsterClass();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterCombatant] WARNING: No valid monster in getMonsterClass!";
        return nullptr;
    }
}

void BattleDialogModelMonsterCombatant::setMonsterName(const QString& monsterName)
{
    if(_monsterName != monsterName)
    {
        _monsterName = monsterName;
        emit dataChanged(this);
    }
}

Monster* BattleDialogModelMonsterCombatant::getMonster() const
{
    return dynamic_cast<Monster*>(_combatant);
}

void BattleDialogModelMonsterCombatant::setMonster(Monster* monster)
{
    setCombatant(monster);
}

void BattleDialogModelMonsterCombatant::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("monsterSize", _monsterSize);
    element.setAttribute("monsterName", _monsterName);
    element.setAttribute("monsterHP", _monsterHP);

    BattleDialogModelMonsterBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
