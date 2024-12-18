#include "battledialogmodelcharacter.h"
#include "characterv2.h"
#include "monsterclassv2.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelCharacter::BattleDialogModelCharacter(const QString& name, QObject *parent) :
    BattleDialogModelCombatant(name, parent)
{
}

BattleDialogModelCharacter::BattleDialogModelCharacter(Characterv2* character) :
    BattleDialogModelCombatant(character)
{
}

BattleDialogModelCharacter::BattleDialogModelCharacter(Characterv2* character, int initiative, const QPointF& position) :
    BattleDialogModelCombatant(character, initiative, position)
{
}

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
    newCharacter->copyValues(this);
    return newCharacter;
}

qreal BattleDialogModelCharacter::getSizeFactor() const
{
    Characterv2* character = getCharacter();
    if(!character)
        return 1;

    return MonsterClass::convertSizeToScaleFactor(character->getStringValue(QString("size")));
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
    Characterv2* character = getCharacter();
    if(!character)
        return 0;

    // HACK
    switch(skill)
    {
        case Combatant::Skills_strengthSave:
            return character->getIntValue(QString("strengthSave"));
        case Combatant::Skills_athletics:
            return character->getIntValue(QString("athletics"));
        case Combatant::Skills_dexteritySave:
            return character->getIntValue(QString("dexteritySave"));
        case Combatant::Skills_stealth:
            return character->getIntValue(QString("stealth"));
        case Combatant::Skills_acrobatics:
            return character->getIntValue(QString("acrobatics"));
        case Combatant::Skills_sleightOfHand:
            return character->getIntValue(QString("sleightOfHand"));
        case Combatant::Skills_constitutionSave:
            return character->getIntValue(QString("constitutionSave"));
        case Combatant::Skills_intelligenceSave:
            return character->getIntValue(QString("intelligenceSave"));
        case Combatant::Skills_investigation:
            return character->getIntValue(QString("investigation"));
        case Combatant::Skills_arcana:
            return character->getIntValue(QString("arcana"));
        case Combatant::Skills_nature:
            return character->getIntValue(QString("nature"));
        case Combatant::Skills_history:
            return character->getIntValue(QString("history"));
        case Combatant::Skills_religion:
            return character->getIntValue(QString("religion"));
        case Combatant::Skills_wisdomSave:
            return character->getIntValue(QString("wisdomSave"));
        case Combatant::Skills_medicine:
            return character->getIntValue(QString("medicine"));
        case Combatant::Skills_animalHandling:
            return character->getIntValue(QString("animalHandling"));
        case Combatant::Skills_perception:
            return character->getIntValue(QString("perception"));
        case Combatant::Skills_insight:
            return character->getIntValue(QString("insight"));
        case Combatant::Skills_survival:
            return character->getIntValue(QString("survival"));
        case Combatant::Skills_charismaSave:
            return character->getIntValue(QString("charismaSave"));
        case Combatant::Skills_performance:
            return character->getIntValue(QString("performance"));
        case Combatant::Skills_deception:
            return character->getIntValue(QString("deception"));
        case Combatant::Skills_persuasion:
            return character->getIntValue(QString("persuasion"));
        case Combatant::Skills_intimidation:
            return character->getIntValue(QString("intimidation"));
        default:
            return 0;
    }
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

bool BattleDialogModelCharacter::hasCondition(Combatant::Condition condition) const
{
    if(_combatant)
    {
        return _combatant->hasCondition(condition);
    }
    else
    {
        qDebug() << "[BattleDialogModelCharacter] WARNING: No valid character in hasCondition!";
        return false;
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

Characterv2* BattleDialogModelCharacter::getCharacter() const
{
    if((!_combatant) || (_combatant->getCombatantType() != DMHelper::CombatantType_Character))
        return nullptr;

    return dynamic_cast<Characterv2*>(_combatant);
}

void BattleDialogModelCharacter::setCharacter(Characterv2* character)
{
    setCombatant(character);
}

void BattleDialogModelCharacter::setConditions(int conditions)
{
    if(_combatant)
    {
        _combatant->setConditions(conditions);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelCharacter::applyConditions(int conditions)
{
    if(_combatant)
    {
        _combatant->applyConditions(conditions);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelCharacter::removeConditions(int conditions)
{
    if(_combatant)
    {
        _combatant->removeConditions(conditions);
        emit conditionsChanged(this);
    }
}

void BattleDialogModelCharacter::clearConditions()
{
    if(_combatant)
    {
        _combatant->clearConditions();
        emit conditionsChanged(this);
    }
}

