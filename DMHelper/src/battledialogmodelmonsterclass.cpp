#include "battledialogmodelmonsterclass.h"
#include "monsterclassv2.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelMonsterClass::BattleDialogModelMonsterClass(const QString& name, QObject *parent) :
    BattleDialogModelMonsterBase(name, parent),
    _monsterClass(nullptr),
    _monsterName(),
    _monsterHP(-1),
    _monsterSize(0.0),
    _iconIndex(0)
{
}

BattleDialogModelMonsterClass::BattleDialogModelMonsterClass(MonsterClassv2* monsterClass) :
    BattleDialogModelMonsterBase(),
    _monsterClass(monsterClass),
    _monsterName(),
    _monsterHP(-1),
    _monsterSize(0.0),
    _iconIndex(0)
{
    if(_monsterClass)
        _monsterHP = _monsterClass->getDiceValue("hit_dice").roll();
}

BattleDialogModelMonsterClass::BattleDialogModelMonsterClass(MonsterClassv2* monsterClass, const QString& monsterName, int initiative, const QPointF& position) :
    BattleDialogModelMonsterBase(nullptr, initiative, position),
    _monsterClass(monsterClass),
    _monsterName(monsterName),
    _monsterHP(-1),
    _monsterSize(0.0),
    _iconIndex(0)
{
    if(_monsterClass)
        _monsterHP = _monsterClass->getDiceValue("hit_dice").roll();
}

BattleDialogModelMonsterClass::~BattleDialogModelMonsterClass()
{
}

void BattleDialogModelMonsterClass::inputXML(const QDomElement &element, bool isImport)
{
    _monsterName = element.attribute("monsterName");
    _monsterHP = element.attribute("monsterHP", QString::number(0)).toInt();
    _monsterSize = element.attribute("monsterSize", QString::number(0.0)).toDouble();
    _iconIndex = element.attribute("iconIndex", QString::number(0)).toInt();
    if((_iconIndex < 0) || ((_iconIndex > 0) && (_monsterClass) && (_iconIndex >= _monsterClass->getIconCount())))
        _iconIndex = 0;

    BattleDialogModelMonsterBase::inputXML(element, isImport);
}

void BattleDialogModelMonsterClass::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelMonsterClass* otherMonsterClass = dynamic_cast<const BattleDialogModelMonsterClass*>(other);
    if(!otherMonsterClass)
        return;

    _monsterClass = otherMonsterClass->_monsterClass;
    _monsterName = otherMonsterClass->_monsterName;
    _monsterHP = otherMonsterClass->_monsterHP;
    _monsterSize = otherMonsterClass->_monsterSize;
    _iconIndex = otherMonsterClass->_iconIndex;

    BattleDialogModelMonsterBase::copyValues(other);
}

BattleDialogModelCombatant* BattleDialogModelMonsterClass::clone() const
{
    BattleDialogModelMonsterClass* newMonster = new BattleDialogModelMonsterClass(getName());

    newMonster->copyValues(this);

    return newMonster;
}

qreal BattleDialogModelMonsterClass::getSizeFactor() const
{
    if(_monsterSize > 0.0)
        return _monsterSize;

    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getSizeFactor!";
        return 1.0;
    }

    return MonsterClassv2::convertSizeToScaleFactor(_monsterClass->getStringValue("size"));
}

int BattleDialogModelMonsterClass::getSizeCategory() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getSizeCategory!";
        return DMHelper::CombatantSize_Medium;
    }

    return MonsterClassv2::convertSizeToCategory(_monsterClass->getStringValue("size"));
}

int BattleDialogModelMonsterClass::getStrength() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getStrength!";
        return 0;
    }

    return _monsterClass->getIntValue("strength");
}

int BattleDialogModelMonsterClass::getDexterity() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getDexterity!";
        return 0;
    }

    return _monsterClass->getIntValue("dexterity");
}

int BattleDialogModelMonsterClass::getConstitution() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getConstitution!";
        return 0;
    }

    return _monsterClass->getIntValue("constitution");
}

int BattleDialogModelMonsterClass::getIntelligence() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getIntelligence!";
        return 0;
    }

    return _monsterClass->getIntValue("intelligence");
}

int BattleDialogModelMonsterClass::getWisdom() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getWisdom!";
        return 0;
    }

    return _monsterClass->getIntValue("wisdom");
}

int BattleDialogModelMonsterClass::getCharisma() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getCharisma!";
        return 0;
    }

    return _monsterClass->getIntValue("charisma");
}

int BattleDialogModelMonsterClass::getSpeed() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getSpeed!";
        return 30;
    }

    return _monsterClass->getIntValue("speed");
}

int BattleDialogModelMonsterClass::getArmorClass() const
{
    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getArmorClass!";
        return 10;
    }

    return _monsterClass->getIntValue("armor_class");
}

int BattleDialogModelMonsterClass::getHitPoints() const
{
    return _monsterHP;
}

void BattleDialogModelMonsterClass::setHitPoints(int hitPoints)
{
    if(_monsterHP != hitPoints)
    {
        _monsterHP = hitPoints;
        emit dataChanged(this);
    }
}

QString BattleDialogModelMonsterClass::getName() const
{
    if(!_monsterName.isEmpty())
        return _monsterName;

    if(!_monsterClass)
    {
        qDebug() << "[BattleDialogModelMonsterClass] WARNING: No valid monster class in getName!";
        return QString();
    }

    return _monsterClass->getStringValue("name");
}

QPixmap BattleDialogModelMonsterClass::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    if(_monsterClass)
    {
        QPixmap result = _monsterClass->getIconPixmap(iconSize, _iconIndex);
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

MonsterClassv2* BattleDialogModelMonsterClass::getMonsterClass() const
{
    return _monsterClass;
}

void BattleDialogModelMonsterClass::setSizeFactor(qreal sizeFactor)
{
    if((_monsterClass) && (MonsterClassv2::convertSizeToScaleFactor(_monsterClass->getStringValue("size")) == sizeFactor))
        return;

    _monsterSize = sizeFactor;
    emit dataChanged(this);
}

void BattleDialogModelMonsterClass::setMonsterName(const QString &monsterName)
{
    if(_monsterName != monsterName)
    {
        _monsterName = monsterName;
        emit dataChanged(this);
    }
}

void BattleDialogModelMonsterClass::setIconIndex(int index)
{
    if((!_monsterClass) || (_iconIndex == index) || (index < 0) || (index >= _monsterClass->getIconCount()))
        return;

    _iconIndex = index;
    emit imageChanged(this);
}

void BattleDialogModelMonsterClass::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("monsterClass", _monsterClass->getStringValue("name"));
    element.setAttribute("monsterName", _monsterName);
    element.setAttribute("monsterHP", _monsterHP);
    if(_monsterSize > 0.0)
        element.setAttribute("monsterSize", _monsterSize);
    if(_iconIndex != 0)
        element.setAttribute("iconIndex", _iconIndex);

    BattleDialogModelMonsterBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
