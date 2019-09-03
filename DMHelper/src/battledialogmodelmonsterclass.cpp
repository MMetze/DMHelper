#include "battledialogmodelmonsterclass.h"
#include "monsterclass.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelMonsterClass::BattleDialogModelMonsterClass() :
    BattleDialogModelMonsterBase(),
    _monsterClass(nullptr),
    _monsterName(),
    _monsterHP(-1)
{
}

BattleDialogModelMonsterClass::BattleDialogModelMonsterClass(MonsterClass* monsterClass) :
    BattleDialogModelMonsterBase(),
    _monsterClass(monsterClass),
    _monsterName(),
    _monsterHP(-1)
{
    if(_monsterClass)
        _monsterHP = _monsterClass->getHitDice().roll();
}

BattleDialogModelMonsterClass::BattleDialogModelMonsterClass(MonsterClass* monsterClass, const QString& monsterName, int initiative, const QPointF& position) :
    BattleDialogModelMonsterBase(nullptr, initiative, position),
    _monsterClass(monsterClass),
    _monsterName(monsterName),
    _monsterHP(-1)
{
    if(_monsterClass)
        _monsterHP = _monsterClass->getHitDice().roll();
}

BattleDialogModelMonsterClass::BattleDialogModelMonsterClass(const BattleDialogModelMonsterClass& other) :
    BattleDialogModelMonsterBase(other),
    _monsterClass(other._monsterClass),
    _monsterName(other._monsterName),
    _monsterHP(other._monsterHP)
{
}

BattleDialogModelMonsterClass::~BattleDialogModelMonsterClass()
{
}

void BattleDialogModelMonsterClass::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelMonsterBase::inputXML(element, isImport);

    _monsterName = element.attribute("monsterName");
    _monsterHP = element.attribute("monsterHP",QString::number(0)).toInt();
}

BattleDialogModelMonsterClass* BattleDialogModelMonsterClass::clone() const
{
    return new BattleDialogModelMonsterClass(*this);
}

int BattleDialogModelMonsterClass::getSizeFactor() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        //return convertSizeToFactor(_monsterClass->getMonsterSize());
        return _monsterClass->getMonsterSizeFactor();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getSizeFactor!";
        return 1;
    }
}

int BattleDialogModelMonsterClass::getSizeCategory() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        //return convertSizeToFactor(_monsterClass->getMonsterSize());
        return _monsterClass->getMonsterSizeCategory();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getSizeCategory!";
        return DMHelper::CombatantSize_Medium;
    }
}

int BattleDialogModelMonsterClass::getStrength() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getStrength();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getStrength!";
        return 0;
    }
}

int BattleDialogModelMonsterClass::getDexterity() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getDexterity();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getDexterity!";
        return 0;
    }
}

int BattleDialogModelMonsterClass::getConstitution() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getConstitution();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getConstitution!";
        return 0;
    }
}

int BattleDialogModelMonsterClass::getIntelligence() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getIntelligence();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getIntelligence!";
        return 0;
    }
}

int BattleDialogModelMonsterClass::getWisdom() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getWisdom();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getWisdom!";
        return 0;
    }
}

int BattleDialogModelMonsterClass::getCharisma() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getCharisma();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getCharisma!";
        return 0;
    }
}

int BattleDialogModelMonsterClass::getSpeed() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getSpeedValue();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getSpeed!";
        return 30;
    }
}

int BattleDialogModelMonsterClass::getArmorClass() const
{
    // TODO: should this just be impossible?
    if(_monsterClass)
    {
        return _monsterClass->getArmorClass();
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getArmorClass!";
        return 10;
    }
}

int BattleDialogModelMonsterClass::getHitPoints() const
{
    return _monsterHP;
}

void BattleDialogModelMonsterClass::setHitPoints(int hitPoints)
{
    _monsterHP = hitPoints;
}

QString BattleDialogModelMonsterClass::getName() const
{
    if(!_monsterName.isEmpty())
    {
        return _monsterName;
    }
    else
    {
        // TODO: should this just be impossible?
        if(_monsterClass)
        {
            return _monsterClass->getName();
        }
        else
        {
            qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getName!";
            return QString();
        }
    }
}

QPixmap BattleDialogModelMonsterClass::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    if(_monsterClass)
    {
        QPixmap result = _monsterClass->getIconPixmap(iconSize);
        if(!result.isNull())
            return result;
        else
            return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
    }
    else
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getIconPixmap!";
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
    }
}

int BattleDialogModelMonsterClass::getMonsterType() const
{
    return BattleMonsterType_Class;
}

MonsterClass* BattleDialogModelMonsterClass::getMonsterClass() const
{
    return _monsterClass;
}

void BattleDialogModelMonsterClass::setMonsterName(const QString &monsterName)
{
    _monsterName = monsterName;
}

void BattleDialogModelMonsterClass::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    BattleDialogModelMonsterBase::internalOutputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("monsterClass", _monsterClass->getName());
    element.setAttribute("monsterName", _monsterName);
    element.setAttribute("monsterHP", _monsterHP);
}
