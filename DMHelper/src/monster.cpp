#include "monster.h"
#include "dmconstants.h"
#include "monsterclass.h"
#include <QDomElement>
#include <QDir>
#include <QDebug>

Monster::Monster(MonsterClass* monsterClass, const QString& name, QObject *parent) :
    Combatant(name, parent),
    _monsterClass(monsterClass),
    _passivePerception(10),
    _active(true),
    _notes(""),
    _iconChanged(false)
{
}

void Monster::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();

    setPassivePerception(element.attribute("passivePerception").toInt());
    setActive(static_cast<bool>(element.attribute("active").toInt()));
    setNotes(element.attribute("notes"));

    Combatant::inputXML(element, isImport);

    endBatchChanges();
}

void Monster::copyValues(const CampaignObjectBase* other)
{
    const Monster* otherMonster = dynamic_cast<const Monster*>(other);
    if(!otherMonster)
        return;

    _monsterClass = otherMonster->_monsterClass;
    _passivePerception = otherMonster->_passivePerception;
    _active = otherMonster->_active;
    _notes = otherMonster->_notes;

    Combatant::copyValues(other);
}

void Monster::beginBatchChanges()
{
    _iconChanged = false;

    Combatant::beginBatchChanges();
}

void Monster::endBatchChanges()
{
    if(_batchChanges)
    {
        if(_iconChanged)
        {
            emit iconChanged();
        }
    }

    Combatant::endBatchChanges();
}

Combatant* Monster::clone() const
{
    if(_monsterClass)
        _monsterClass->searchForIcons();

    Monster* newMonster = new Monster(_monsterClass, getName());

    newMonster->copyValues(this);

    newMonster->_monsterClass = _monsterClass;
    newMonster->_passivePerception = _passivePerception;
    newMonster->_active = _active;
    newMonster->_notes = _notes;
    newMonster->_iconChanged = _iconChanged;

    return newMonster;
}

int Monster::getCombatantType() const
{
    return DMHelper::CombatantType_Monster;
}

int Monster::getSpeed() const
{
    if(_monsterClass)
        return _monsterClass->getSpeedValue();
    else
        return 0;
}

int Monster::getArmorClass() const
{
    return _monsterClass ? _monsterClass->getArmorClass() : Combatant::getArmorClass();
}

Dice Monster::getHitDice() const
{
    return _monsterClass ? _monsterClass->getHitDice() : Combatant::getHitDice();
}

MonsterClass* Monster::getMonsterClass() const
{
    return _monsterClass;
}

QString Monster::getMonsterClassName() const
{
    return _monsterClass ? _monsterClass->getName() : QString("");
}

QString Monster::getIcon(bool localOnly) const
{
    if((!localOnly) && (_icon.isEmpty()) && (_monsterClass))
    {
        return _monsterClass->getIcon();
    }
    else
    {
        return Combatant::getIcon();
    }
}

QPixmap Monster::getIconPixmap(DMHelper::PixmapSize iconSize)
{
    if((_icon.isEmpty())&&(_monsterClass))
    {
        QPixmap result = _monsterClass->getIconPixmap(iconSize);
        if(!result.isNull())
        {
            return result;
        }
        else
        {
            return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
        }
    }
    else
    {
        return Combatant::getIconPixmap(iconSize);
    }
}

int Monster::getStrength() const
{
    return _monsterClass ? _monsterClass->getStrength() : 10;
}

int Monster::getDexterity() const
{
    return _monsterClass ? _monsterClass->getDexterity() : 10;
}

int Monster::getConstitution() const
{
    return _monsterClass ? _monsterClass->getConstitution() : 10;
}

int Monster::getIntelligence() const
{
    return _monsterClass ? _monsterClass->getIntelligence() : 10;
}

int Monster::getWisdom() const
{
    return _monsterClass ? _monsterClass->getWisdom() : 10;
}

int Monster::getCharisma() const
{
    return _monsterClass ? _monsterClass->getCharisma() : 10;
}

int Monster::getPassivePerception() const
{
    return _passivePerception;
}

bool Monster::getActive() const
{
    return _active;
}

QString Monster::getNotes() const
{
    return _notes;
}

void Monster::setMonsterClass(MonsterClass* newMonsterClass)
{
    if(!newMonsterClass || (newMonsterClass == _monsterClass))
        return;

    _monsterClass = newMonsterClass;
    registerChange();
}

void Monster::setIcon(const QString& newIcon)
{
    if(newIcon == _icon)
        return;

    _icon = newIcon;
    _iconPixmap.setBasePixmap(_icon);
    qDebug() << "Monster has a local icon:" << getName() << ": " << _icon;
    registerChange();
    if(_batchChanges)
    {
        _iconChanged = true;
    }
    else
    {
        emit iconChanged();
    }
}

void Monster::setPassivePerception(int value)
{
    if(value == _passivePerception)
        return;

    _passivePerception = value;
    registerChange();
}

void Monster::setActive(bool value)
{
    if(value == _active)
        return;

    _active = value;
    registerChange();
}

void Monster::setNotes(const QString& newNotes)
{
    if(newNotes == _notes)
        return;

    _notes = newNotes;
    registerChange();
}

void Monster::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute( "monsterClass", getMonsterClass() != nullptr ? getMonsterClass()->getName() : QString("") );
    element.setAttribute( "passivePerception", getPassivePerception() );
    element.setAttribute( "active", static_cast<int>(getActive()));
    element.setAttribute( "notes", getNotes() );

    Combatant::internalOutputXML(doc, element, targetDirectory, isExport);
}
