#include "combatant.h"
#include "dmconstants.h"
#include "scaledpixmap.h"
#include "monster.h"
#include <QDomElement>
#include <QDomDocument>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QDebug>

const Combatant::Condition CONDITION_ITERATOR_VALUES[Combatant::Condition_Iterator_Count] =
{
    Combatant::Condition_None,          // Condition_Iterator_None
    Combatant::Condition_Blinded,       // Condition_Iterator_Blinded
    Combatant::Condition_Charmed,       // Condition_Iterator_Charmed
    Combatant::Condition_Deafened,      // Condition_Iterator_Deafened
    Combatant::Condition_Exhaustion_1,  // Condition_Iterator_Exhaustion_1
    Combatant::Condition_Exhaustion_2,  // Condition_Iterator_Exhaustion_2
    Combatant::Condition_Exhaustion_3,  // Condition_Iterator_Exhaustion_3
    Combatant::Condition_Exhaustion_4,  // Condition_Iterator_Exhaustion_4
    Combatant::Condition_Exhaustion_5,  // Condition_Iterator_Exhaustion_5
    Combatant::Condition_Frightened,    // Condition_Iterator_Frightened
    Combatant::Condition_Grappled,      // Condition_Iterator_Grappled
    Combatant::Condition_Incapacitated, // Condition_Iterator_Incapacitated
    Combatant::Condition_Invisible,     // Condition_Iterator_Invisible
    Combatant::Condition_Paralyzed,     // Condition_Iterator_Paralyzed
    Combatant::Condition_Petrified,     // Condition_Iterator_Petrified
    Combatant::Condition_Poisoned,      // Condition_Iterator_Poisoned
    Combatant::Condition_Prone,         // Condition_Iterator_Prone
    Combatant::Condition_Restrained,    // Condition_Iterator_Restrained
    Combatant::Condition_Stunned,       // Condition_Iterator_Stunned
    Combatant::Condition_Unconscious    // Condition_Iterator_Unconscious
};

Combatant::Combatant(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _initiative(0),
    _armorClass(10),
    _attacks(),
    _hitPoints(0),
    _hitDice(),
    _conditions(Condition_None),
    _icon(""),
    _iconPixmap(),
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
    setConditions(element.attribute("conditions", QString("0")).toInt());
    setInitiative(element.attribute("initiative", QString("0")).toInt());
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
    _conditions = otherCombatant->_conditions;
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
        {
            emit dirty();
        }
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

QString Combatant::getIcon(bool localOnly) const
{
    Q_UNUSED(localOnly);
    return _icon;
}

QPixmap Combatant::getIconPixmap(DMHelper::PixmapSize iconSize)
{
    if(_iconPixmap.isValid())
        return _iconPixmap.getPixmap(iconSize);
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
    return convertModToStr(getAbilityMod(ability));
}

QString Combatant::convertModToStr(int modifier)
{
    QString modifierStr = QString::number(modifier);
    if( modifier >= 0 )
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

int Combatant::getConditionCount()
{
    return Combatant::Condition_Iterator_Count;
}

Combatant::Condition Combatant::getConditionByIndex(int index)
{
    if((index < 0) || (index >= Condition_Iterator_Count))
        return Condition_None;
    else
        return CONDITION_ITERATOR_VALUES[index];
}

QString Combatant::getConditionIcon(int condition)
{
    switch(condition)
    {
        case Condition_Blinded: return QString("one-eyed");
        case Condition_Charmed: return QString("smitten");
        case Condition_Deafened: return QString("elf-ear");
        case Condition_Exhaustion_1: return QString("crawl");
        case Condition_Exhaustion_2: return QString("crawl");
        case Condition_Exhaustion_3: return QString("crawl");
        case Condition_Exhaustion_4: return QString("crawl");
        case Condition_Exhaustion_5: return QString("crawl");
        case Condition_Frightened: return QString("sharp-smile");
        case Condition_Grappled: return QString("grab");
        case Condition_Incapacitated: return QString("internal-injury");
        case Condition_Invisible: return QString("invisible");
        case Condition_Paralyzed: return QString("aura");
        case Condition_Petrified: return QString("stone-pile");
        case Condition_Poisoned: return QString("deathcab");
        case Condition_Prone: return QString("falling");
        case Condition_Restrained: return QString("imprisoned");
        case Condition_Stunned: return QString("embrassed-energy");
        case Condition_Unconscious: return QString("coma");
        default: return QString();
    }
}

QString Combatant::getConditionTitle(int condition)
{
    switch(condition)
    {
        case Condition_Blinded: return QString("Blinded");
        case Condition_Charmed: return QString("Charmed");
        case Condition_Deafened: return QString("Deafened");
        case Condition_Exhaustion_1:
        case Condition_Exhaustion_2:
        case Condition_Exhaustion_3:
        case Condition_Exhaustion_4:
        case Condition_Exhaustion_5: return QString("Exhaustion");
        case Condition_Frightened: return QString("Frightened");
        case Condition_Grappled: return QString("Grappled");
        case Condition_Incapacitated: return QString("Incapacitated");
        case Condition_Invisible: return QString("Invisible");
        case Condition_Paralyzed: return QString("Paralyzed");
        case Condition_Petrified: return QString("Petrified");
        case Condition_Poisoned: return QString("Poisoned");
        case Condition_Prone: return QString("Prone");
        case Condition_Restrained: return QString("Restrained");
        case Condition_Stunned: return QString("Stunned");
        case Condition_Unconscious: return QString("Unconscious");
        default: return QString();
    }
}

QString Combatant::getConditionDescription(int condition)
{
    switch(condition)
    {
        case Condition_Blinded: return QString("Blinded");
        case Condition_Charmed: return QString("Charmed");
        case Condition_Deafened: return QString("Deafened");
        case Condition_Exhaustion_1: return QString("Exhaustion - Level 1");
        case Condition_Exhaustion_2: return QString("Exhaustion - Level 2");
        case Condition_Exhaustion_3: return QString("Exhaustion - Level 3");
        case Condition_Exhaustion_4: return QString("Exhaustion - Level 4");
        case Condition_Exhaustion_5: return QString("Exhaustion - Level 5");
        case Condition_Frightened: return QString("Frightened");
        case Condition_Grappled: return QString("Grappled");
        case Condition_Incapacitated: return QString("Incapacitated");
        case Condition_Invisible: return QString("Invisible");
        case Condition_Paralyzed: return QString("Paralyzed");
        case Condition_Petrified: return QString("Petrified");
        case Condition_Poisoned: return QString("Poisoned");
        case Condition_Prone: return QString("Prone");
        case Condition_Restrained: return QString("Restrained");
        case Condition_Stunned: return QString("Stunned");
        case Condition_Unconscious: return QString("Unconscious");
        default: return QString();
    }
}

void Combatant::drawConditions(QPaintDevice* target, int conditions)
{
    if((!target) || (conditions == Condition_None))
        return;

    int spacing = target->width() / 20;
    int iconSize = (target->width() / 3) - spacing;
    if((spacing <= 0) || (iconSize <= 5) || (iconSize + spacing >= target->height()))
    {
        qDebug() << "[Combatant] spacing or icon size are not ok to draw conditions. Spacing: " << spacing << ", icon size: " << iconSize << ", target: " << target->width() << " x " << target->height();
        return;
    }

    QPainter painter(target);
    int cx = spacing;
    int cy = spacing;
    for(int i = 0; i < Combatant::getConditionCount(); ++i)
    {
        int condition = Combatant::getConditionByIndex(i);
        if((conditions & condition) && (cy <= target->height() - iconSize))
        {
            // QPixmap conditionPixmap(QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png"));
            // painter.drawPixmap(cx, cy, conditionPixmap.scaled(iconSize, iconSize));
            QPixmap conditionPixmap(QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png"));
            /*
            QImage coloredIcon(iconSize, iconSize, QImage::Format_ARGB32);
            coloredIcon.fill(Qt::yellow);
            QPainter iconPainter;
            iconPainter.begin(&coloredIcon);
                iconPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                iconPainter.drawPixmap(QPoint(0, 0), conditionPixmap.scaled(iconSize, iconSize));
            iconPainter.end();
            painter.drawImage(cx, cy, coloredIcon);
            */
            painter.drawPixmap(cx, cy, conditionPixmap.scaled(iconSize, iconSize));
            cx += iconSize + spacing;
            if(cx > target->width() - iconSize)
            {
                cx = spacing;
                cy += iconSize;
            }
        }
    }
}

QStringList Combatant::getConditionString(int conditions)
{
    QStringList result;

    if(conditions != Condition_None)
    {
        for(int i = 0; i < Combatant::getConditionCount(); ++i)
        {
            int condition = Combatant::getConditionByIndex(i);
            if(conditions & condition)
            {
                result.append(Combatant::getConditionDescription(condition));
            }
        }
    }

    return result;
}

int Combatant::getConditions() const
{
    return _conditions;
}

bool Combatant::hasCondition(Condition condition) const
{
    return ((_conditions & condition) != 0);
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

void Combatant::setConditions(int conditions)
{
    if(_conditions != conditions)
    {
        _conditions = conditions;
        registerChange();
    }
}

void Combatant::applyConditions(int conditions)
{
    if((_conditions & conditions) != conditions)
    {
        _conditions |= conditions;
        registerChange();
    }
}

void Combatant::removeConditions(int conditions)
{
    if((_conditions & ~conditions) != _conditions)
    {
        _conditions &= ~conditions;
        registerChange();
    }
}

void Combatant::addCondition(Condition condition)
{
    if(!hasCondition(condition))
    {
        _conditions |= condition;
        registerChange();
    }
}

void Combatant::removeCondition(Condition condition)
{
    if(hasCondition(condition))
    {
        _conditions &= ~condition;
        registerChange();
    }
}

void Combatant::clearConditions()
{
    setConditions(Condition_None);
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

QDomElement Combatant::createOutputXML(QDomDocument &doc)
{
    return doc.createElement( "combatant" );
}

void Combatant::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("type", getCombatantType());
    element.setAttribute("armorClass", getArmorClass());
    element.setAttribute("hitPoints", getHitPoints());
    element.setAttribute("hitDice", getHitDice().toString());
    element.setAttribute("conditions", getConditions());
    element.setAttribute("initiative", getInitiative());

    QString iconPath = getIcon(true);
    if(iconPath.isEmpty())
    {
        element.setAttribute("icon", QString(""));
    }
    else
    {
        element.setAttribute("icon", targetDirectory.relativeFilePath(iconPath));
    }

    QDomElement attacksElement = doc.createElement("attacks");
    for(int i = 0; i < getAttacks().count(); ++i)
    {
        QDomElement attackElement = doc.createElement("attack");
        attackElement.setAttribute("name", getAttacks().at(i).getName());
        attackElement.setAttribute("dice", getAttacks().at(i).getDice().toString());
        attacksElement.appendChild(attackElement);
    }
    element.appendChild(attacksElement);

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
    {
        _changesMade = true;
    }
    else
    {
        emit dirty();
    }
}
