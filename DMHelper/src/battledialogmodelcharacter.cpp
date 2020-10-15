#include "battledialogmodelcharacter.h"
#include "character.h"
#include "monsterclass.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelCharacter::BattleDialogModelCharacter(const QString& name, QObject *parent) :
    BattleDialogModelCombatant(name, parent)
{
}

BattleDialogModelCharacter::BattleDialogModelCharacter(Character* character) :
    BattleDialogModelCombatant(character)
{
}

BattleDialogModelCharacter::BattleDialogModelCharacter(Character* character, int initiative, const QPointF& position) :
    BattleDialogModelCombatant(character, initiative, position)
{
}

/*
BattleDialogModelCharacter::BattleDialogModelCharacter(const BattleDialogModelCharacter& other) :
    BattleDialogModelCombatant(other)
{
}
*/

BattleDialogModelCharacter::~BattleDialogModelCharacter()
{
}

void BattleDialogModelCharacter::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelCombatant::inputXML(element, isImport);
}

int BattleDialogModelCharacter::getCombatantType() const
{
    return DMHelper::CombatantType_Character;
}

BattleDialogModelCombatant* BattleDialogModelCharacter::clone() const
{
    BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(getName());
    newCharacter->copyValues(*this);
    return newCharacter;
}

qreal BattleDialogModelCharacter::getSizeFactor() const
{
    Character* character = getCharacter();
    if(!character)
        return 1;

    return MonsterClass::convertSizeToScaleFactor(character->getStringValue(Character::StringValue_size));
}

int BattleDialogModelCharacter::getSizeCategory() const
{
    return DMHelper::CombatantSize_Medium;
}

int BattleDialogModelCharacter::getStrength() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getStrength();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getStrength!";
        return 0;
    }
}

int BattleDialogModelCharacter::getDexterity() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getDexterity();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getDexterity!";
        return 0;
    }
}

int BattleDialogModelCharacter::getConstitution() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getConstitution();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getConstitution!";
        return 0;
    }
}

int BattleDialogModelCharacter::getIntelligence() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getIntelligence();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getIntelligence!";
        return 0;
    }
}

int BattleDialogModelCharacter::getWisdom() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getWisdom();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getWisdom!";
        return 0;
    }
}

int BattleDialogModelCharacter::getCharisma() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getCharisma();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getCharisma!";
        return 0;
    }
}

int BattleDialogModelCharacter::getSkillModifier(Combatant::Skills skill) const
{
    Character* character = getCharacter();
    if(!character)
        return 0;

    int modifier = Combatant::getAbilityMod(_combatant->getAbilityValue(Combatant::getSkillAbility(skill)));
    if(character->getSkillValue(skill))
        modifier += character->getProficiencyBonus();

    return modifier;
}

int BattleDialogModelCharacter::getConditions() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getConditions();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getConditions!";
        return 0;
    }
}

int BattleDialogModelCharacter::getSpeed() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getSpeed();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getSpeed!";
        return 30;
    }
}

int BattleDialogModelCharacter::getArmorClass() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getArmorClass();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getArmorClass!";
        return 10;
    }
}

int BattleDialogModelCharacter::getHitPoints() const
{
    if(_combatant)
    {
        return _combatant->getHitPoints();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getHitPoints!";
        return 0;
    }
}

void BattleDialogModelCharacter::setHitPoints(int hitPoints)
{
    if(_combatant)
    {
        _combatant->setHitPoints(hitPoints);
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in setHitPoints!";
    }
}

QString BattleDialogModelCharacter::getName() const
{
    if(_combatant)
    {
        return _combatant->getName();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getName!";
        return QString();
    }
}

QPixmap BattleDialogModelCharacter::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    if(_combatant)
    {
        return _combatant->getIconPixmap(iconSize);
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in getIconPixmap!";
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
    }
}

Character* BattleDialogModelCharacter::getCharacter() const
{
    if((!_combatant) || (_combatant->getCombatantType() != DMHelper::CombatantType_Character))
        return nullptr;

    return dynamic_cast<Character*>(_combatant);
}

void BattleDialogModelCharacter::setCharacter(Character* character)
{
    setCombatant(character);
}

void BattleDialogModelCharacter::setConditions(int conditions)
{
    if(_combatant)
        _combatant->setConditions(conditions);
}

void BattleDialogModelCharacter::applyConditions(int conditions)
{
    if(_combatant)
        _combatant->applyConditions(conditions);
}
