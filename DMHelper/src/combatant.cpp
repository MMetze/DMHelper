#include "combatant.h"
#include "dmconstants.h"
#include "scaledpixmap.h"
#include "monster.h"
#include <QDomElement>
#include <QDomDocument>
#include <QDir>
#include <QPixmap>

Combatant::Combatant(QObject *parent) :
    CampaignObjectBase(parent),
    _name(""),
    _initiative(0),
    _armorClass(10),
    _attacks(),
    _hitPoints(0),
    _hitDice(),
    _icon(""),
    _iconPixmap(),
    _batchChanges(false),
    _changesMade(false)
{
}

Combatant::Combatant(const Combatant &obj) :
    CampaignObjectBase(obj.parent()),
    _name(obj._name),
    _initiative(obj._initiative),
    _armorClass(obj._armorClass),
    _hitPoints(obj._hitPoints),
    _hitDice(obj._hitDice.toString()),
    _icon(obj._icon),
    _iconPixmap(obj._iconPixmap),
    _batchChanges(obj._batchChanges),
    _changesMade(obj._changesMade)
{
    for(int i = 0; i < obj._attacks.count(); ++i)
    {
        _attacks.append(obj._attacks.at(i));
    }
}

Combatant::~Combatant()
{
}

void Combatant::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement element = doc.createElement( "combatant" );

    CampaignObjectBase::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute( "name", getName() );
    element.setAttribute( "type", getType() );
    element.setAttribute( "armorClass", getArmorClass() );
    element.setAttribute( "hitPoints", getHitPoints() );
    element.setAttribute( "hitDice", getHitDice().toString() );

    QString iconPath = getIcon(true);
    if(iconPath.isEmpty())
    {
        element.setAttribute( "icon", QString("") );
    }
    else
    {
        element.setAttribute( "icon", targetDirectory.relativeFilePath(iconPath));
    }

    QDomElement attacksElement = doc.createElement( "attacks" );
    for(int i = 0; i < getAttacks().count(); ++i)
    {
        QDomElement attackElement = doc.createElement( "attack" );
        attackElement.setAttribute( "name", getAttacks().at(i).getName() );
        attackElement.setAttribute( "dice", getAttacks().at(i).getDice().toString() );
        attacksElement.appendChild(attackElement);
    }
    element.appendChild(attacksElement);

    internalOutputXML(doc, element, targetDirectory, isExport);

    parent.appendChild(element);
}

void Combatant::inputXML(const QDomElement &element, bool isImport)
{
    CampaignObjectBase::inputXML(element, isImport);

    setName(element.attribute("name"));
    setArmorClass(element.attribute("armorClass").toInt());
    setHitPoints(element.attribute("hitPoints").toInt());
    setHitDice(Dice(element.attribute("hitDice")));
    setIcon(element.attribute("icon"));

    QDomElement attacksElement = element.firstChildElement( QString("attacks") );
    if( !attacksElement.isNull() )
    {
        QDomElement attackElement = attacksElement.firstChildElement( QString("attack") );
        while( !attackElement.isNull() )
        {
            addAttack(Attack(attackElement.attribute("name"), attackElement.attribute("dice")));
            attackElement = attackElement.nextSiblingElement( QString("attack") );
        }
    }
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
        {
            emit dirty();
        }
    }
}

QString Combatant::getName() const
{
    return _name;
}

int Combatant::getType() const
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

QString Combatant::getIcon(bool localOnly) const
{
    Q_UNUSED(localOnly);
    return _icon;
}

QPixmap Combatant::getIconPixmap(DMHelper::PixmapSize iconSize)
{
    QPixmap result = _iconPixmap.getPixmap(iconSize);
    if(!result.isNull())
        return result;
    else
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
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
    int abilityMod = getAbilityMod(ability);

    QString abilityModStr = QString::number(abilityMod);
    if( abilityMod >= 0 )
        abilityModStr.prepend("+");

    return abilityModStr;
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
        if( combatantGroup.first > 1) {
            newCombatant->setName(baseName + QString("#") + QString::number(n+1));
        } else {
            newCombatant->setName(baseName);
        }
        if( newCombatant->getHitPoints() == 0 )
        {
            newCombatant->setHitPoints(newCombatant->getHitDice().roll());
        }
        newCombatant->setInitiative(Dice::d20() + Combatant::getAbilityMod(combatantGroup.second->getDexterity()));

        result.append(newCombatant);
    }

    return result;
}

void Combatant::setName(const QString& combatantName)
{
    if(combatantName != _name)
    {
        _name = combatantName;
        //registerChange();
        emit changed();
    }
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

void Combatant::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(element);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport)
}

void Combatant::registerChange()
{
    if(_batchChanges)
    {
        _changesMade = true;
    }
    else
    {
        emit dirty();
    }
}
