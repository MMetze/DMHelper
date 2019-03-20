#include "monsterclass.h"
#include "dmconstants.h"
#include "dice.h"
#include "bestiary.h"
#include "character.h"
#include <QDomElement>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

static const char* SKILLELEMEMT_NAMES[Combatant::SKILLS_COUNT] =
{
    "strength_save",     // Skills_strengthSave
    "athletics",         // Skills_athletics
    "dexterity_save",    // Skills_dexteritySave
    "stealth",           // Skills_stealth
    "acrobatics",        // Skills_acrobatics
    "sleight_of_hand",   // Skills_sleightOfHand
    "constitution_save", // Skills_constitutionSave
    "intelligence_save", // Skills_intelligenceSave
    "investigation",     // Skills_investigation
    "arcana",            // Skills_arcana
    "nature",            // Skills_nature
    "history",           // Skills_history
    "religion",          // Skills_religion
    "wisdom_save",       // Skills_wisdomSave
    "medicine",          // Skills_medicine
    "animal_handling",   // Skills_animalHandling
    "perception",        // Skills_perception
    "insight",           // Skills_insight
    "survival",          // Skills_survival
    "charisma_save",     // Skills_charismaSave
    "performance",       // Skills_performance
    "deception",         // Skills_deception
    "persuasion",        // Skills_persuasion
    "intimidation"       // Skills_intimidation
};

MonsterClass::MonsterClass(const QString& name, QObject *parent) :
    QObject(parent),
    _private(false),
    _icon(""),
    _name(name),
    _monsterType("Beast"),
    _monsterSubType(),
    _monsterSize("medium"),
    _monsterSizeCategory(DMHelper::CombatantSize_Medium),
    _speed("30 ft"),
    _alignment("neutral"),
    _languages("---"),
    _armorClass(10),
    _hitDice("1d8"),
    _averageHitPoints(4),
    _conditionImmunities(""),
    _damageImmunities(""),
    _damageResistances(""),
    _damageVulnerabilities(""),
    _senses(""),
    _challenge("1"),
    _skillValues(),
    _strength(10),
    _dexterity(10),
    _constitution(10),
    _intelligence(10),
    _wisdom(10),
    _charisma(10),
    _batchChanges(false),
    _changesMade(false),
    _iconChanged(false),
    _scaledPixmap()
{
}

MonsterClass::MonsterClass(const QDomElement &element, QObject *parent) :
    QObject(parent),
    _private(false),
    _icon(""),
    _name(""),
    _monsterType("Beast"),
    _monsterSubType(),
    _monsterSize("medium"),
    _monsterSizeCategory(DMHelper::CombatantSize_Medium),
    _speed("30 ft"),
    _alignment("neutral"),
    _languages("---"),
    _armorClass(10),
    _hitDice("1d8"),
    _averageHitPoints(4),
    _conditionImmunities(""),
    _damageImmunities(""),
    _damageResistances(""),
    _damageVulnerabilities(""),
    _senses(""),
    _challenge("1"),
    _skillValues(),
    _strength(10),
    _dexterity(10),
    _constitution(10),
    _intelligence(10),
    _wisdom(10),
    _charisma(10),
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

    setPrivate(static_cast<bool>(element.attribute("private",QString::number(0)).toInt()));
    setIcon(element.attribute("icon"));

    setName(element.firstChildElement(QString("name")).text());
    setMonsterType(element.firstChildElement(QString("type")).text());
    setMonsterSubType(element.firstChildElement(QString("subtype")).text());
    setMonsterSize(element.firstChildElement(QString("size")).text());
    setSpeed(element.firstChildElement(QString("speed")).text());
    setAlignment(element.firstChildElement(QString("alignment")).text());
    setLanguages(element.firstChildElement(QString("languages")).text());
    setArmorClass(element.firstChildElement(QString("armor_class")).text().toInt());
    setHitDice(Dice(element.firstChildElement(QString("hit_dice")).text()));
    setAverageHitPoints(element.firstChildElement(QString("hit_points")).text().toInt());
    setConditionImmunities(element.firstChildElement(QString("condition_immunities")).text());
    setDamageImmunities(element.firstChildElement(QString("damage_immunities")).text());
    setDamageResistances(element.firstChildElement(QString("damage_resistances")).text());
    setDamageVulnerabilities(element.firstChildElement(QString("damage_vulnerabilities")).text());
    setSenses(element.firstChildElement(QString("senses")).text());
    setChallenge(element.firstChildElement(QString("challenge_rating")).text());
    setStrength(element.firstChildElement(QString("strength")).text().toInt());
    setDexterity(element.firstChildElement(QString("dexterity")).text().toInt());
    setConstitution(element.firstChildElement(QString("constitution")).text().toInt());
    setIntelligence(element.firstChildElement(QString("intelligence")).text().toInt());
    setWisdom(element.firstChildElement(QString("wisdom")).text().toInt());
    setCharisma(element.firstChildElement(QString("charisma")).text().toInt());

    for(int s = Combatant::Skills_strengthSave; s < Combatant::SKILLS_COUNT; ++s)
        checkForSkill(element, SKILLELEMEMT_NAMES[s], static_cast<Combatant::Skills>(s));

    readActionList(element, QString("actions"), _actions);
    readActionList(element, QString("legendary_actions"), _legendaryActions);
    readActionList(element, QString("special_abilities"), _specialAbilities);
    readActionList(element, QString("reactions"), _reactions);

    endBatchChanges();
}

void MonsterClass::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) const
{
    element.setAttribute( "private", static_cast<int>(getPrivate()) );

    QString iconPath = getIcon();
    if(iconPath.isEmpty())
    {
        element.setAttribute( "icon", QString("") );
    }
    else
    {
        element.setAttribute( "icon", targetDirectory.relativeFilePath(iconPath));
    }

    outputValue(doc, element, QString("name"), getName());
    outputValue(doc, element, QString("type"), getMonsterType());
    outputValue(doc, element, QString("subtype"), getMonsterSubType());
    outputValue(doc, element, QString("size"), getMonsterSize());
    outputValue(doc, element, QString("speed"), getSpeed());
    outputValue(doc, element, QString("alignment"), getAlignment());
    outputValue(doc, element, QString("languages"), getLanguages());
    outputValue(doc, element, QString("armor_class"), QString::number(getArmorClass()));
    outputValue(doc, element, QString("hit_dice"), getHitDice().toString());
    outputValue(doc, element, QString("hit_points"), QString::number(getAverageHitPoints()));
    outputValue(doc, element, QString("condition_immunities"), getConditionImmunities());
    outputValue(doc, element, QString("damage_immunities"), getDamageImmunities());
    outputValue(doc, element, QString("damage_resistances"), getDamageResistances());
    outputValue(doc, element, QString("damage_vulnerabilities"), getDamageVulnerabilities());
    outputValue(doc, element, QString("senses"), getSenses());
    outputValue(doc, element, QString("challenge_rating"), getChallenge());
    outputValue(doc, element, QString("strength"), QString::number(getStrength()));
    outputValue(doc, element, QString("dexterity"), QString::number(getDexterity()));
    outputValue(doc, element, QString("constitution"), QString::number(getConstitution()));
    outputValue(doc, element, QString("intelligence"), QString::number(getIntelligence()));
    outputValue(doc, element, QString("wisdom"), QString::number(getWisdom()));
    outputValue(doc, element, QString("charisma"), QString::number(getCharisma()));

    for(int s = Combatant::Skills_strengthSave; s < Combatant::SKILLS_COUNT; ++s)
    {
        if(_skillValues.contains(static_cast<Combatant::Skills>(s)))
            outputValue(doc, element, SKILLELEMEMT_NAMES[s], QString::number(_skillValues[s]));
    }

    writeActionList(doc, element, QString("actions"), _actions);
    writeActionList(doc, element, QString("legendary_actions"), _legendaryActions);
    writeActionList(doc, element, QString("special_abilities"), _specialAbilities);
    writeActionList(doc, element, QString("reactions"), _reactions);

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
    return _legendaryActions.count() > 0;
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

QString MonsterClass::getMonsterSubType() const
{
    return _monsterSubType;
}

QString MonsterClass::getMonsterSize() const
{
    return _monsterSize;
}

int MonsterClass::getMonsterSizeCategory() const
{
    return _monsterSizeCategory;
}

int MonsterClass::getMonsterSizeFactor() const
{
    return convertSizeCategoryToScaleFactor(getMonsterSizeCategory());
}

QString MonsterClass::getSpeed() const
{
    return _speed;
}

int MonsterClass::getSpeedValue() const
{
    QString speedStr = getSpeed();
    return speedStr.left(speedStr.indexOf(" ")).toInt();
}

QString MonsterClass::getAlignment() const
{
    return _alignment;
}

QString MonsterClass::getLanguages() const
{
    return _languages;
}

int MonsterClass::getArmorClass() const
{
    return _armorClass;
}

Dice MonsterClass::getHitDice() const
{
    return _hitDice;
}

int MonsterClass::getAverageHitPoints() const
{
    return _averageHitPoints;
}

QString MonsterClass::getConditionImmunities() const
{
    return _conditionImmunities;
}

QString MonsterClass::getDamageImmunities() const
{
    return _damageImmunities;
}

QString MonsterClass::getDamageResistances() const
{
    return _damageResistances;
}

QString MonsterClass::getDamageVulnerabilities() const
{
    return _damageVulnerabilities;
}

QString MonsterClass::getSenses() const
{
    return _senses;
}

QString MonsterClass::getChallenge() const
{
    return _challenge;
}

float MonsterClass::getChallengeNumber() const
{
    QStringList challengeParts = _challenge.split('/');
    if(challengeParts.count() == 1)
        return challengeParts.at(0).toFloat();

    if((challengeParts.count() == 2) && (challengeParts.at(1).toFloat() > 0.f))
        return challengeParts.at(0).toFloat() / challengeParts.at(1).toFloat();

    return 0;
}

int MonsterClass::getXP() const
{
    return convertCRtoXP(getChallengeNumber());
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

int MonsterClass::getAbilityValue(Combatant::Ability ability) const
{
    if(Combatant::Ability_Strength == ability)
        return getStrength();
    else if(Combatant::Ability_Dexterity == ability)
        return getDexterity();
    else if(Combatant::Ability_Constitution == ability)
        return getConstitution();
    else if(Combatant::Ability_Intelligence == ability)
        return getIntelligence();
    else if(Combatant::Ability_Wisdom == ability)
        return getWisdom();
    else if(Combatant::Ability_Charisma == ability)
        return getCharisma();
    else
        return -1;
}

int MonsterClass::getSkillValue(Combatant::Skills skill) const
{
    if(_skillValues.contains(skill))
        return _skillValues.value(skill, 0);
    else
        return Combatant::getAbilityMod(getAbilityValue(Combatant::getSkillAbility(skill)));
}

QList<MonsterAction> MonsterClass::getActions() const
{
    return _actions;
}

void MonsterClass::addAction(const MonsterAction& action)
{
    //MonsterAction newAction(action);
    //_actions.append(newAction);
    _actions.append(action);
}

void MonsterClass::setAction(int index, const MonsterAction& action)
{
    if((index < 0) || (index >= _actions.count()))
        return;

    if(_actions.at(index) != action)
        _actions[index] = action;
}

int MonsterClass::removeAction(const MonsterAction& action)
{
    _actions.removeAll(action);
    return _actions.count();
}

QList<MonsterAction> MonsterClass::getLegendaryActions() const
{
    return _legendaryActions;
}

void MonsterClass::addLegendaryAction(const MonsterAction& action)
{
    //MonsterAction newAction(action);
    //_legendaryActions.append(newAction);
    _legendaryActions.append(action);
}

void MonsterClass::setLegendaryAction(int index, const MonsterAction& action)
{
    if((index < 0) || (index >= _legendaryActions.count()))
        return;

    if(_legendaryActions.at(index) != action)
        _legendaryActions[index] = action;
}

int MonsterClass::removeLegendaryAction(const MonsterAction& action)
{
    _legendaryActions.removeAll(action);
    return _legendaryActions.count();
}

QList<MonsterAction> MonsterClass::getSpecialAbilities() const
{
    return _specialAbilities;
}

void MonsterClass::addSpecialAbility(const MonsterAction& action)
{
    //MonsterAction newAction(action);
    //_specialAbilities.append(newAction);
    _specialAbilities.append(action);
}

void MonsterClass::setSpecialAbility(int index, const MonsterAction& action)
{
    if((index < 0) || (index >= _specialAbilities.count()))
        return;

    if(_specialAbilities.at(index) != action)
        _specialAbilities[index] = action;
}

int MonsterClass::removeSpecialAbility(const MonsterAction& action)
{
    _specialAbilities.removeAll(action);
    return _specialAbilities.count();
}

QList<MonsterAction> MonsterClass::getReactions() const
{
    return _reactions;
}

void MonsterClass::addReaction(const MonsterAction& action)
{
    //MonsterAction newAction(action);
    //_reactions.append(newAction);
    _reactions.append(action);
}

void MonsterClass::setReaction(int index, const MonsterAction& action)
{
    if((index < 0) || (index >= _reactions.count()))
        return;

    if(_reactions.at(index) != action)
        _reactions[index] = action;
}

int MonsterClass::removeReaction(const MonsterAction& action)
{
    _reactions.removeAll(action);
    return _reactions.count();
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

void MonsterClass::outputValue(QDomDocument &doc, QDomElement &element, const QString& valueName, const QString& valueText)
{
    QDomElement newChild = doc.createElement(valueName);
    newChild.appendChild(doc.createTextNode(valueText));
    element.appendChild(newChild);
}

void MonsterClass::setPrivate(bool isPrivate)
{
    if(isPrivate == _private)
        return;

    _private = isPrivate;
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
    registerChange();
}

void MonsterClass::setMonsterSubType(const QString& monsterSubType)
{
    if(monsterSubType == _monsterSubType)
        return;

    _monsterSubType = monsterSubType;
    registerChange();
}

void MonsterClass::setMonsterSize(const QString& monsterSize)
{
    if(monsterSize == _monsterSize)
        return;

    _monsterSize = monsterSize;
    _monsterSizeCategory = convertSizeToCategory(getMonsterSize());
    registerChange();
}

void MonsterClass::setSpeed(const QString& speed)
{
    if(speed == _speed)
        return;

    _speed = speed;
    registerChange();
}

void MonsterClass::setAlignment(const QString& alignment)
{
    if(alignment == _alignment)
        return;

    _alignment = alignment;
    registerChange();
}

void MonsterClass::setLanguages(const QString& languages)
{
    if(languages == _languages)
        return;

    _languages = languages;
    registerChange();
}

void MonsterClass::setArmorClass(int armorClass)
{
    if(armorClass == _armorClass)
        return;

    _armorClass = armorClass;
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

    if((_averageHitPoints != _hitDice.average()) && (_hitDice.getBonus() == 0))
    {
        _hitDice = Dice(_hitDice.getCount(), _hitDice.getType(), _averageHitPoints - _hitDice.average());
    }

    registerChange();
}

void MonsterClass::setConditionImmunities(const QString& conditionImmunities)
{
    if(conditionImmunities == _conditionImmunities)
        return;

    _conditionImmunities = conditionImmunities;
    registerChange();
}

void MonsterClass::setDamageImmunities(const QString& damageImmunities)
{
    if(damageImmunities == _damageImmunities)
        return;

    _damageImmunities = damageImmunities;
    registerChange();
}

void MonsterClass::setDamageResistances(const QString& damageResistances)
{
    if(damageResistances == _damageResistances)
        return;

    _damageResistances = damageResistances;
    registerChange();
}

void MonsterClass::setDamageVulnerabilities(const QString& damageVulnerabilities)
{
    if(damageVulnerabilities == _damageVulnerabilities)
        return;

    _damageVulnerabilities = damageVulnerabilities;
    registerChange();
}

void MonsterClass::setSenses(const QString& senses)
{
    if(senses == _senses)
        return;

    _senses = senses;
    registerChange();
}

void MonsterClass::setChallenge(const QString& challenge)
{
    if(challenge == _challenge)
        return;

    _challenge = challenge;
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

void MonsterClass::calculateHitDiceBonus()
{
    int newBonus = _averageHitPoints - _hitDice.average();
    _hitDice = Dice(_hitDice.getCount(), _hitDice.getType(), newBonus);
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

void MonsterClass::checkForSkill(const QDomElement& element, const QString& skillName, Combatant::Skills skill)
{
    QDomElement skillElement = element.firstChildElement(skillName);
    if(skillElement.isNull())
        return;

    _skillValues[skill] = skillElement.text().toInt();
}

void MonsterClass::readActionList(const QDomElement& element, const QString& actionName, QList<MonsterAction>& actionList)
{
    QDomElement actionListElement = element.firstChildElement(actionName);
    if(actionListElement.isNull())
        return;

    QDomElement actionElement = actionListElement.firstChildElement("element");
    while(!actionElement.isNull())
    {
        MonsterAction newAction(actionElement);
        actionList.append(newAction);
        actionElement = actionElement.nextSiblingElement("element");
    }
}

void MonsterClass::writeActionList(QDomDocument &doc, QDomElement& element, const QString& actionName, const QList<MonsterAction>& actionList) const
{
    QDomElement actionListElement = doc.createElement(actionName);

    for(int i = 0; i < actionList.count(); ++i)
    {
        QDomElement actionElement = doc.createElement("element");
        actionList.at(i).outputXML(doc, actionElement);
        actionListElement.appendChild(actionElement);
    }

    element.appendChild(actionListElement);
}

int MonsterClass::convertCRtoXP(float challengeRating)
{
    if(challengeRating >= 30.f)
        return 155000;
    else if(challengeRating >= 29.f)
        return 135000;
    else if(challengeRating >= 28.f)
        return 120000;
    else if(challengeRating >= 27.f)
        return 105000;
    else if(challengeRating >= 26.f)
        return 90000;
    else if(challengeRating >= 25.f)
        return 75000;
    else if(challengeRating >= 24.f)
        return 62000;
    else if(challengeRating >= 23.f)
        return 50000;
    else if(challengeRating >= 22.f)
        return 31000;
    else if(challengeRating >= 21.f)
        return 33000;
    else if(challengeRating >= 20.f)
        return 25000;
    else if(challengeRating >= 19.f)
        return 22000;
    else if(challengeRating >= 18.f)
        return 20000;
    else if(challengeRating >= 17.f)
        return 18000;
    else if(challengeRating >= 16.f)
        return 15000;
    else if(challengeRating >= 15.f)
        return 13000;
    else if(challengeRating >= 14.f)
        return 11500;
    else if(challengeRating >= 13.f)
        return 10000;
    else if(challengeRating >= 12.f)
        return 8400;
    else if(challengeRating >= 11.f)
        return 7200;
    else if(challengeRating >= 10.f)
        return 5900;
    else if(challengeRating >= 9.f)
        return 5000;
    else if(challengeRating >= 8.f)
        return 3900;
    else if(challengeRating >= 7.f)
        return 2900;
    else if(challengeRating >= 6.f)
        return 2300;
    else if(challengeRating >= 5.f)
        return 1800;
    else if(challengeRating >= 4.f)
        return 1100;
    else if(challengeRating >= 3.f)
        return 700;
    else if(challengeRating >= 2.f)
        return 450;
    else if(challengeRating >= 1.f)
        return 200;
    else if(challengeRating >= 0.5f)
        return 100;
    else if(challengeRating >= 0.25f)
        return 50;
    else if(challengeRating >= 0.125f)
        return 25;
    else
        return 10;
}
