#include "battledialogmodelcharacter.h"
//#include "character.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelCharacter::BattleDialogModelCharacter() :
    BattleDialogModelCombatant()
{
}

BattleDialogModelCharacter::BattleDialogModelCharacter(int initiative, const QPointF& position) :
    BattleDialogModelCombatant(initiative, position)
{
}

BattleDialogModelCharacter::BattleDialogModelCharacter(const BattleDialogModelCharacter& other) :
    BattleDialogModelCombatant(other)
{
}

BattleDialogModelCharacter::~BattleDialogModelCharacter()
{
}

void BattleDialogModelCharacter::inputXML(const QDomElement &element)
{
    BattleDialogModelCombatant::inputXML(element);
}

int BattleDialogModelCharacter::getType() const
{
    return DMHelper::CombatantType_Character;
}

BattleDialogModelCombatant* BattleDialogModelCharacter::clone() const
{
    return new BattleDialogModelCharacter(*this);
}

int BattleDialogModelCharacter::getSizeFactor() const
{
    return 1;
}

int BattleDialogModelCharacter::getSizeCategory() const
{
    return DMHelper::CombatantSize_Medium;
}

/*
int BattleDialogModelCharacter::getStrength() const
{
    // TODO: should this just be impossible?
    if(_combatant)
    {
        return _combatant->getStrength();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] No valid character in getStrength!";
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in getDexterity!";
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in getConstitution!";
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in getIntelligence!";
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in getWisdom!";
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in getCharisma!";
        return 0;
    }
}

int BattleDialogModelCharacter::getSkillModifier(Combatant::Skills skill) const
{
    if((!_combatant) || (_combatant->getType() != DMHelper::CombatantType_Character))
        return 0;

    Character* character = dynamic_cast<Character*>(_combatant);
    if(!character)
        return 0;

    int modifier = Combatant::getAbilityMod(_combatant->getAbilityValue(Combatant::getSkillAbility(skill)));
    if(character->getSkillValue(skill))
        modifier += character->getProficiencyBonus();

    return modifier;
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in getArmorClass!";
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in getHitPoints!";
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
        qDebug() << "[BattleDialogModelCharacter] No valid character in setHitPoints!";
    }
}
*/

QString BattleDialogModelCharacter::getName() const
{
    // TODO: get name
    /*
    if(_combatant)
    {
        return _combatant->getName();
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] No valid character in getName!";
        return QString();
    }
    */
    return QString();
}

/*
QPixmap BattleDialogModelCharacter::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    // TODO: Get Pixmap
    if(_combatant)
    {
        return _combatant->getIconPixmap(iconSize);
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] No valid character in getIconPixmap!";
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
    }
}
*/

/*
Character* BattleDialogModelCharacter::getCharacter() const
{
    return dynamic_cast<Character*>(_combatant);
}

void BattleDialogModelCharacter::setCharacter(Character* character)
{
    setCombatant(character);
}

void BattleDialogModelCharacter::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    Q_UNUSED(doc);
    Q_UNUSED(element);
    Q_UNUSED(targetDirectory);
}
*/
