#include "combatant.h"
#include "conditions.h"
#include "dmconstants.h"
#include "scaledpixmap.h"
#include "monster.h"
#include <QDomElement>
#include <QDomDocument>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QDebug>

Combatant::Combatant(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _initiative(0),
    _armorClass(10),
    _attacks(),
    _hitPoints(0),
    _hitDice(),
    _conditionList(),
    _icon(""),
    _iconPixmap(),
    _backgroundColor(Qt::black),
    _batchChanges(false),
    _changesMade(false)
{
}

Combatant::~Combatant()
{
}

void Combatant::inputXML(const QDomElement &element, bool isImport)
{
    setArmorClass(element.attribute("armorClass").toInt());
    setHitPoints(element.attribute("hitPoints").toInt());
    setHitDice(Dice(element.attribute("hitDice")));

    // Condition migration: detect old int bitmask format vs new comma-separated string IDs
    QString condStr = element.attribute("conditions", QString());
    if(!condStr.isEmpty())
    {
        bool ok = false;
        int condInt = condStr.toInt(&ok);
        if(ok)
            _conditionList = Conditions::migrateFromBitmask(condInt);
        else
            _conditionList = condStr.split(QStringLiteral(","), Qt::SkipEmptyParts);
    }

    setInitiative(element.attribute("initiative", QString("0")).toInt());
    setIcon(element.attribute("icon"));
    setBackgroundColor(QColor(element.attribute("backgroundColor", QString("#000000"))));

    QDomElement attacksElement = element.firstChildElement(QString("attacks"));
    if(!attacksElement.isNull())
    {
        QDomElement attackElement = attacksElement.firstChildElement(QString("attack"));
        while(!attackElement.isNull())
        {
            addAttack(Attack(attackElement.attribute("name"), attackElement.attribute("dice")));
            attackElement = attackElement.nextSiblingElement(QString("attack"));
        }
    }

    CampaignObjectBase::inputXML(element, isImport);
}

void Combatant::copyValues(const CampaignObjectBase* other)
{
    const Combatant* otherCombatant = dynamic_cast<const Combatant*>(other);
    if(!otherCombatant)
        return;

    _initiative = otherCombatant->_initiative;
    _armorClass = otherCombatant->_armorClass;
    _hitPoints = otherCombatant->_hitPoints;
    _hitDice = otherCombatant->_hitDice;
    _conditionList = otherCombatant->_conditionList;
    _icon = otherCombatant->_icon;
    _iconPixmap.setBasePixmap(_icon);

    for(int i = 0; i < otherCombatant->_attacks.count(); ++i)
    {
        _attacks.append(Attack(otherCombatant->_attacks.at(i)));
    }

    CampaignObjectBase::copyValues(other);
}

int Combatant::getObjectType() const
{
    return DMHelper::CampaignType_Combatant;
}

void Combatant::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
}

void Combatant::endBatchChanges()
{
    if(_batchChanges)
    {
        _batchChanges = false;
        if(_changesMade)
            emit dirty();
    }
}

int Combatant::getCombatantType() const
{
    return DMHelper::CombatantType_Base;
}

int Combatant::getInitiative() const
{
    return _initiative;
}

int Combatant::getArmorClass() const
{
    return _armorClass;
}

QList<Attack> Combatant::getAttacks() const
{
    return _attacks;
}

int Combatant::getHitPoints() const
{
    return _hitPoints;
}

Dice Combatant::getHitDice() const
{
    return _hitDice;
}

QString Combatant::getIconFile() const
{
    return _icon;
}

QString Combatant::getIconFileLocal() const
{
    return _icon;
}

QPixmap Combatant::getIconPixmap(DMHelper::PixmapSize iconSize)
{
    if(_iconPixmap.isValid())
        return _iconPixmap.getPixmap(iconSize);
    else
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
}

QColor Combatant::getBackgroundColor() const
{
    return _backgroundColor;
}

int Combatant::getAbilityValue(Ability ability) const
{
    switch(ability)
    {
        case Ability_Strength:
            return getStrength();
        case Ability_Dexterity:
            return getDexterity();
        case Ability_Constitution:
            return getConstitution();
        case Ability_Intelligence:
            return getIntelligence();
        case Ability_Wisdom:
            return getWisdom();
        case Ability_Charisma:
            return getCharisma();
        default:
            return -1;
    }
}

int Combatant::getAbilityMod(int ability)
{
    switch(ability)
    {
        case 1: return -5;
        case 2: case 3: return -4;
        case 4: case 5: return -3;
        case 6: case 7: return -2;
        case 8: case 9: return -1;
        case 10: case 11: return 0;
        case 12: case 13: return 1;
        case 14: case 15: return 2;
        case 16: case 17: return 3;
        case 18: case 19: return 4;
        case 20: case 21: return 5;
        case 22: case 23: return 6;
        case 24: case 25: return 7;
        case 26: case 27: return 8;
        case 28: case 29: return 9;
        default: return 10;
    }
}

QString Combatant::getAbilityModStr(int ability)
{
    return convertModToStr(getAbilityMod(ability));
}

QString Combatant::convertModToStr(int modifier)
{
    QString modifierStr = QString::number(modifier);
    if(modifier >= 0)
        modifierStr.prepend("+");

    return modifierStr;
}

Combatant::Ability Combatant::getSkillAbility(Skills skill)
{
    switch(skill)
    {
        case Skills_strengthSave: case Skills_athletics:
            return Ability_Strength;
        case Skills_dexteritySave: case Skills_stealth: case Skills_acrobatics: case Skills_sleightOfHand:
            return Ability_Dexterity;
        case Skills_constitutionSave:
            return Ability_Constitution;
        case Skills_intelligenceSave: case Skills_investigation: case Skills_arcana: case Skills_nature: case Skills_history: case Skills_religion:
            return Ability_Intelligence;
        case Skills_wisdomSave: case Skills_medicine: case Skills_animalHandling: case Skills_perception: case Skills_insight: case Skills_survival:
            return Ability_Wisdom;
        case Skills_charismaSave: case Skills_performance: case Skills_deception: case Skills_persuasion: case Skills_intimidation:
            return Ability_Charisma;
        default:
            return Ability_Strength;
    }
}

bool Combatant::isSkillSavingThrow(Skills skill)
{
    return ((skill == Skills_strengthSave) ||
            (skill == Skills_dexteritySave) ||
            (skill == Skills_constitutionSave) ||
            (skill == Skills_intelligenceSave) ||
            (skill == Skills_wisdomSave) ||
            (skill == Skills_charismaSave));
}

/*
QList<Combatant*> Combatant::instantiateCombatants(CombatantGroup combatantGroup)
{
    // TODO: Will be obsolete
    QList<Combatant*> result;

    if(!combatantGroup.second)
        return result;

    QString baseName = combatantGroup.second->getName();
    if(baseName.isEmpty())
    {
        Monster* monster = dynamic_cast<Monster*>(combatantGroup.second);
        if(monster)
        {
            baseName = monster->getMonsterClassName();
        }
    }

    for(int n = 0; n < combatantGroup.first; ++n)
    {
        Combatant* newCombatant = combatantGroup.second->clone();
        if(combatantGroup.first > 1) {
            newCombatant->setName(baseName + QString("#") + QString::number(n+1));
        } else {
            newCombatant->setName(baseName);
        }
        if(newCombatant->getHitPoints() == 0)
        {
            newCombatant->setHitPoints(newCombatant->getHitDice().roll());
        }
        newCombatant->setInitiative(Dice::d20() + Combatant::getAbilityMod(combatantGroup.second->getDexterity()));

        result.append(newCombatant);
    }

    return result;
}
*/

QStringList Combatant::getConditionList() const
{
    return _conditionList;
}

bool Combatant::hasConditionId(const QString& conditionId) const
{
    return _conditionList.contains(conditionId);
}

void Combatant::setInitiative(int initiative)
{
    if(initiative != _initiative)
    {
        _initiative = initiative;
    }
}

void Combatant::setArmorClass(int armorClass)
{
    if(armorClass != _armorClass)
    {
        _armorClass = armorClass;
        registerChange();
    }
}

void Combatant::addAttack(const Attack& attack)
{
    _attacks.append(attack);
    registerChange();
}

void Combatant::removeAttack(int index)
{
    if((index >= 0) && (index < _attacks.count()))
    {
        _attacks.removeAt(index);
        registerChange();
    }
}

void Combatant::setHitPoints(int hitPoints)
{
    if(hitPoints != _hitPoints)
    {
        _hitPoints = hitPoints;
        registerChange();
    }
}

void Combatant::setConditionList(const QStringList& conditions)
{
    if(_conditionList != conditions)
    {
        _conditionList = conditions;
        registerChange();
    }
}

void Combatant::addConditionId(const QString& conditionId)
{
    if(!conditionId.isEmpty() && !_conditionList.contains(conditionId))
    {
        _conditionList.append(conditionId);
        registerChange();
    }
}

void Combatant::removeConditionId(const QString& conditionId)
{
    if(_conditionList.removeOne(conditionId))
    {
        registerChange();
    }
}

void Combatant::clearConditions()
{
    if(!_conditionList.isEmpty())
    {
        _conditionList.clear();
        registerChange();
    }
}

void Combatant::applyDamage(int damage)
{
    if(damage <= 0)
        return;

    if(_hitPoints > damage)
        _hitPoints -= damage;
    else
        _hitPoints = 0;

    registerChange();
}

void Combatant::setHitDice(const Dice& hitDice)
{
    if(hitDice != _hitDice)
    {
        _hitDice = hitDice;
        registerChange();
    }
}

void Combatant::setIcon(const QString &newIcon)
{
    if(newIcon != _icon)
    {
        _icon = newIcon;
        _iconPixmap.setBasePixmap(_icon);
        registerChange();
    }
}

void Combatant::setBackgroundColor(const QColor &color)
{
    if(color != _backgroundColor)
    {
        _backgroundColor = color;
        registerChange();
    }
}

QDomElement Combatant::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("combatant");
}

void Combatant::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("type", getCombatantType());
    element.setAttribute("armorClass", getArmorClass());
    element.setAttribute("hitPoints", getHitPoints());
    element.setAttribute("hitDice", getHitDice().toString());
    if(!_conditionList.isEmpty())
        element.setAttribute("conditions", _conditionList.join(QStringLiteral(",")));
    element.setAttribute("initiative", getInitiative());

    if((_backgroundColor.isValid()) && (_backgroundColor != Qt::black))
        element.setAttribute("backgroundColor", _backgroundColor.name());

    QString iconPath = getIconFileLocal();
    if(iconPath.isEmpty())
        element.setAttribute("icon", QString(""));
    else
        element.setAttribute("icon", targetDirectory.relativeFilePath(iconPath));

    if(getAttacks().count() > 0)
    {
        QDomElement attacksElement = doc.createElement("attacks");
        for(int i = 0; i < getAttacks().count(); ++i)
        {
            QDomElement attackElement = doc.createElement("attack");
            attackElement.setAttribute("name", getAttacks().at(i).getName());
            attackElement.setAttribute("dice", getAttacks().at(i).getDice().toString());
            attacksElement.appendChild(attackElement);
        }
        element.appendChild(attacksElement);
    }

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Combatant::belongsToObject(QDomElement& element)
{
    if(element.tagName() == QString("attacks"))
        return true;
    else
        return CampaignObjectBase::belongsToObject(element);
}

void Combatant::registerChange()
{
    if(_batchChanges)
        _changesMade = true;
    else
        emit dirty();
}
