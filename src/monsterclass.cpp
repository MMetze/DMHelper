#include "monsterclass.h"
#include "dmconstants.h"
#include "dice.h"
#include "bestiary.h"
#include "character.h"
#include <QDomElement>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

MonsterClass::MonsterClass(const QString& name, QObject *parent) :
    QObject(parent),
    _private(false),
    _legendary(false),
    _icon(""),
    _name(name),
    _monsterType("Beast"),
    _monsterSizeCategory(DMHelper::CombatantSize_Medium),
    _alignment("neutral"),
    _armorClass(10),
    _armorClassDescription("natural"),
    _hitDice("1d8"),
    _averageHitPoints(4),
    _speed("30 ft"),
    _strength(10),
    _dexterity(10),
    _constitution(10),
    _intelligence(10),
    _wisdom(10),
    _charisma(10),
    _skills(""),
    _skillValues(),
    _resistances(""),
    _senses("Passive Perception 10"),
    _languages("---"),
    _challenge(1.f),
    _XP(100),
    _traits("TBD"),
    _actions("TBD"),
    _batchChanges(false),
    _changesMade(false),
    _iconChanged(false),
    _scaledPixmap()
{
}

MonsterClass::MonsterClass(QDomElement &element, QObject *parent) :
    QObject(parent),
    _private(false),
    _legendary(false),
    _icon(""),
    _name(""),
    _monsterType("Beast"),
    _monsterSizeCategory(DMHelper::CombatantSize_Medium),
    _alignment("neutral"),
    _armorClass(10),
    _armorClassDescription("natural"),
    _hitDice("1d8"),
    _averageHitPoints(4),
    _speed("30 ft"),
    _strength(10),
    _dexterity(10),
    _constitution(10),
    _intelligence(10),
    _wisdom(10),
    _charisma(10),
    _skills(""),
    _skillValues(),
    _resistances(""),
    _senses("Passive Perception 10"),
    _languages("---"),
    _challenge(1.f),
    _XP(100),
    _traits("TBD"),
    _actions("TBD"),
    _batchChanges(false),
    _changesMade(false),
    _iconChanged(false),
    _scaledPixmap()
{
    inputXML(element);
}

void MonsterClass::inputXML(const QDomElement &element)
{
    beginBatchChanges();

    /*
     * Useful, but only needed once to convert monster names from all-caps to capitalized words
     *
    QString inputName = element.attribute("name");
    QStringList splitName = inputName.split(" ");
    QString uppedName;
    for(int i = 0; i < splitName.count(); ++i)
    {
        if(i > 0)
            uppedName.append(QString(" "));

        QString namePart = splitName.at(i);
        uppedName.append(namePart.left(1).toUpper());
        uppedName.append(namePart.mid(1).toLower());
    }
    setName(uppedName);
    */
    setName(element.attribute("name"));

    setPrivate((bool)(element.attribute("private",QString::number(0)).toInt()));
    setLegendary((bool)(element.attribute("legendary",QString::number(0)).toInt()));
    setIcon(element.attribute("icon"));
    setMonsterType(element.attribute("monsterType"));
    setAlignment(element.attribute("alignment"));
    setArmorClass(element.attribute("armorClass").toInt());
    setArmorClassDescription(element.attribute("armorClassDescription"));
    setHitDice(Dice(element.attribute("hitDice")));
    setAverageHitPoints(element.attribute("averageHitPoints").toInt());
    setSpeed(element.attribute("speed"));
    setStrength(element.attribute("strength").toInt());
    setDexterity(element.attribute("dexterity").toInt());
    setConstitution(element.attribute("constitution").toInt());
    setIntelligence(element.attribute("intelligence").toInt());
    setWisdom(element.attribute("wisdom").toInt());
    setCharisma(element.attribute("charisma").toInt());
    setSkills(element.attribute("skills"));
    setResistances(element.attribute("resistance"));
    setSenses(element.attribute("senses"));
    setLanguages(element.attribute("languages"));
    setChallenge(element.attribute("challenge").toFloat());
    setXP(element.attribute("xp").toInt());

    QDomElement traitsElement = element.firstChildElement( QString("traits") );
    QDomCDATASection traitsData = traitsElement.firstChild().toCDATASection();
    setTraits(traitsData.data());

    QDomElement actionsElement = element.firstChildElement( QString("actions") );
    QDomCDATASection actionsData = actionsElement.firstChild().toCDATASection();
    setActions(actionsData.data());

    endBatchChanges();
}

void MonsterClass::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) const
{
    QString iconPath = getIcon();
    if(iconPath.isEmpty())
    {
        element.setAttribute( "icon", QString("") );
    }
    else
    {
        element.setAttribute( "icon", targetDirectory.relativeFilePath(iconPath));
    }

    element.setAttribute( "name", getName() );
    element.setAttribute( "private", (int)getPrivate() );
    element.setAttribute( "legendary", (int)getLegendary() );
    element.setAttribute( "monsterType", getMonsterType() );
    element.setAttribute( "alignment", getAlignment() );
    element.setAttribute( "armorClass", getArmorClass() );
    element.setAttribute( "armorClassDescription", getArmorClassDescription() );
    element.setAttribute( "hitDice", getHitDice().toString() );
    element.setAttribute( "averageHitPoints", getAverageHitPoints() );
    element.setAttribute( "speed", getSpeed() );
    element.setAttribute( "strength", getStrength() );
    element.setAttribute( "dexterity", getDexterity() );
    element.setAttribute( "constitution", getConstitution() );
    element.setAttribute( "intelligence", getIntelligence() );
    element.setAttribute( "wisdom", getWisdom() );
    element.setAttribute( "charisma", getCharisma() );
    element.setAttribute( "skills", getSkills() );
    element.setAttribute( "resistance", getResistances() );
    element.setAttribute( "senses", getSenses() );
    element.setAttribute( "languages", getLanguages() );
    element.setAttribute( "challenge", getChallenge() );
    element.setAttribute( "xp", getXP() );

    QDomElement traitsElement = doc.createElement( "traits" );
    QDomCDATASection traitsData = doc.createCDATASection(getTraits());
    traitsElement.appendChild(traitsData);
    element.appendChild(traitsElement);

    QDomElement actionsElement = doc.createElement( "actions" );
    QDomCDATASection actionsData = doc.createCDATASection(getActions());
    actionsElement.appendChild(actionsData);
    element.appendChild(actionsElement);
}

void MonsterClass::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
    _iconChanged = false;
}

void MonsterClass::endBatchChanges()
{
    if(_batchChanges)
    {
        _batchChanges = false;
        if(_iconChanged)
        {
            emit iconChanged();
        }
        if(_changesMade)
        {
            emit dirty();
        }
    }
}

int MonsterClass::getType() const
{
    return DMHelper::CombatantType_Monster;
}

bool MonsterClass::getPrivate() const
{
    return _private;
}

bool MonsterClass::getLegendary() const
{
    return _legendary;
}

QString MonsterClass::getIcon() const
{
    return _icon;
}

QPixmap MonsterClass::getIconPixmap(DMHelper::PixmapSize iconSize)
{
    if(!_scaledPixmap.isValid())
    {
        searchForIcon(QString());
    }

    return _scaledPixmap.getPixmap(iconSize);
}

QString MonsterClass::getName() const
{
    return _name;
}

QString MonsterClass::getMonsterType() const
{
    return _monsterType;
}

QString MonsterClass::getMonsterSize() const
{
    return _monsterType.split(" ").first();
}

int MonsterClass::getMonsterSizeCategory() const
{
    return _monsterSizeCategory;
}

int MonsterClass::getMonsterSizeFactor() const
{
    return convertSizeCategoryToScaleFactor(getMonsterSizeCategory());
}

QString MonsterClass::getAlignment() const
{
    return _alignment;
}

int MonsterClass::getArmorClass() const
{
    return _armorClass;
}

QString MonsterClass::getArmorClassDescription() const
{
    return _armorClassDescription;
}

Dice MonsterClass::getHitDice() const
{
    return _hitDice;
}

int MonsterClass::getAverageHitPoints() const
{
    return _averageHitPoints;
}

QString MonsterClass::getSpeed() const
{
    return _speed;
}

int MonsterClass::getStrength() const
{
    return _strength;
}

int MonsterClass::getDexterity() const
{
    return _dexterity;
}

int MonsterClass::getConstitution() const
{
    return _constitution;
}

int MonsterClass::getIntelligence() const
{
    return _intelligence;
}

int MonsterClass::getWisdom() const
{
    return _wisdom;
}

int MonsterClass::getCharisma() const
{
    return _charisma;
}

QString MonsterClass::getSkills() const
{
    return _skills;
}

QString MonsterClass::getResistances() const
{
    return _resistances;
}

QString MonsterClass::getSenses() const
{
    return _senses;
}

QString MonsterClass::getLanguages() const
{
    return _languages;
}

float MonsterClass::getChallenge() const
{
    return _challenge;
}

int MonsterClass::getXP() const
{
    return _XP;
}

QString MonsterClass::getTraits() const
{
    return _traits;
}

QString MonsterClass::getActions() const
{
    return _actions;
}

int MonsterClass::getAbilityValue(Combatant::Ability ability) const
{
    switch(ability)
    {
        case Combatant::Ability_Strength:
            return getStrength();
        case Combatant::Ability_Dexterity:
            return getDexterity();
        case Combatant::Ability_Constitution:
            return getConstitution();
        case Combatant::Ability_Intelligence:
            return getIntelligence();
        case Combatant::Ability_Wisdom:
            return getWisdom();
        case Combatant::Ability_Charisma:
            return getCharisma();
        default:
            return -1;
    }
}

int MonsterClass::getSkillValue(Combatant::Skills skill) const
{
    if(_skillValues.contains(skill))
        return _skillValues.value(skill, 0);
    else
        return Combatant::getAbilityMod(getAbilityValue(Combatant::getSkillAbility(skill)));
}

void MonsterClass::setPrivate(bool isPrivate)
{
    if(isPrivate == _private)
        return;

    _private = isPrivate;
    registerChange();
}

void MonsterClass::setLegendary(bool isLegendary)
{
    if(isLegendary == _legendary)
        return;

    _legendary = isLegendary;
    registerChange();
}

void MonsterClass::setIcon(const QString &newIcon)
{
    if(newIcon == _icon)
        return;

    searchForIcon(newIcon);
}

void MonsterClass::searchForIcon(const QString &newIcon)
{
    QString searchResult = Bestiary::Instance()->findMonsterImage(getName(),newIcon);
    if(!searchResult.isEmpty())
    {
        _icon = searchResult;
        _scaledPixmap.setBasePixmap(Bestiary::Instance()->getDirectory().filePath(_icon));
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
}

void MonsterClass::setName(const QString& name)
{
    if(name == _name)
        return;

    _name = name;
    registerChange();
}

void MonsterClass::setMonsterType(const QString& monsterType)
{
    if(monsterType == _monsterType)
        return;

    _monsterType = monsterType;
    _monsterSizeCategory = convertSizeToCategory(getMonsterSize());
    registerChange();
}

void MonsterClass::setAlignment(const QString& alignment)
{
    if(alignment == _alignment)
        return;

    _alignment = alignment;
    registerChange();
}

void MonsterClass::setArmorClass(int armorClass)
{
    if(armorClass == _armorClass)
        return;

    _armorClass = armorClass;
    registerChange();
}

void MonsterClass::setArmorClassDescription(const QString& armorClassDescription)
{
    if(armorClassDescription == _armorClassDescription)
        return;

    _armorClassDescription = armorClassDescription;
    registerChange();
}

void MonsterClass::setHitDice(const Dice& hitDice)
{
    if(hitDice == _hitDice)
        return;

    _hitDice = hitDice;
    registerChange();
}

void MonsterClass::setAverageHitPoints(int averageHitPoints)
{
    if(averageHitPoints == _averageHitPoints)
        return;

    _averageHitPoints = averageHitPoints;
    registerChange();
}

void MonsterClass::setSpeed(const QString& speed)
{
    if(speed == _speed)
        return;

    _speed = speed;
    registerChange();
}

void MonsterClass::setStrength(int score)
{
    if(score == _strength)
        return;

    _strength = score;
    registerChange();
}

void MonsterClass::setDexterity(int score)
{
    if(score == _dexterity)
        return;

    _dexterity = score;
    registerChange();
}

void MonsterClass::setConstitution(int score)
{
    if(score == _constitution)
        return;

    _constitution = score;
    registerChange();
}

void MonsterClass::setIntelligence(int score)
{
    if(score == _intelligence)
        return;

    _intelligence = score;
    registerChange();
}

void MonsterClass::setWisdom(int score)
{
    if(score == _wisdom)
        return;

    _wisdom = score;
    registerChange();
}

void MonsterClass::setCharisma(int score)
{
    if(score == _charisma)
        return;

    _charisma = score;
    registerChange();
}

void MonsterClass::setSkills(const QString& skills)
{
    if(skills == _skills)
        return;

    _skills = skills;

    QRegularExpression re("[, ]+");
    QStringList parts = _skills.split(QRegularExpression("[, ]+"), QString::SkipEmptyParts);
    if(!parts.empty())
    {
        int i = 0;
        while(i < parts.count())
        {
            int boolKey = Character::findKeyForSkillName(parts.at(i));
            if(boolKey >= 0)
            {
                int modifier = parts.at(++i).toInt();
                _skillValues[boolKey] = modifier;
            }
            ++i;
        }
    }

    registerChange();
}

void MonsterClass::setResistances(const QString& resistances)
{
    if(resistances == _resistances)
        return;

    _resistances = resistances;
    registerChange();
}

void MonsterClass::setSenses(const QString& senses)
{
    if(senses == _senses)
        return;

    _senses = senses;
    registerChange();
}

void MonsterClass::setLanguages(const QString& languages)
{
    if(languages == _languages)
        return;

    _languages = languages;
    registerChange();
}

void MonsterClass::setChallenge(float challenge)
{
    if(challenge == _challenge)
        return;

    _challenge = challenge;
    registerChange();
}

void MonsterClass::setXP(int xp)
{
    if(xp == _XP)
        return;

    _XP = xp;
    registerChange();
}

void MonsterClass::setTraits(const QString& traits)
{
    if(traits == _traits)
        return;

    _traits = traits;
    registerChange();
}

void MonsterClass::setActions(const QString& actions)
{
    if(actions == _actions)
        return;

    _actions = actions;
    registerChange();
}

void MonsterClass::registerChange()
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

int MonsterClass::convertSizeToCategory(const QString& monsterSize)
{
    if(monsterSize == QString("Tiny"))
        return DMHelper::CombatantSize_Tiny;
    else if(monsterSize == QString("Small"))
        return DMHelper::CombatantSize_Small;
    else if(monsterSize == QString("Medium"))
        return DMHelper::CombatantSize_Medium;
    else if(monsterSize == QString("Large"))
        return DMHelper::CombatantSize_Large;
    else if(monsterSize == QString("Huge"))
        return DMHelper::CombatantSize_Huge;
    else if(monsterSize == QString("Gargantuan"))
        return DMHelper::CombatantSize_Gargantuan;
    else
        return DMHelper::CombatantSize_Unknown;
}

int MonsterClass::convertSizeCategoryToScaleFactor(int category)
{
    switch(category)
    {
        case DMHelper::CombatantSize_Tiny:
            return 1;
        case DMHelper::CombatantSize_Small:
            return 1;
        case DMHelper::CombatantSize_Medium:
            return 1;
        case DMHelper::CombatantSize_Large:
            return 2;
        case DMHelper::CombatantSize_Huge:
            return 3;
        case DMHelper::CombatantSize_Gargantuan:
            return 4;
        default:
            return 1;
    }
}

int MonsterClass::convertSizeToScaleFactor(const QString& monsterSize)
{
    return convertSizeCategoryToScaleFactor(convertSizeToCategory(monsterSize));
}
