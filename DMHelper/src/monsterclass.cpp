#include "monsterclass.h"
#include "dmconstants.h"
#include "dice.h"
#include "bestiary.h"
#include "character.h"
#include <QDomElement>
#include <QDir>
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
    _icons(),
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
    _scaledPixmaps()
{
}

MonsterClass::MonsterClass(const QDomElement &element, bool isImport, QObject *parent) :
    QObject(parent),
    _private(false),
    _icons(),
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
    _actions(),
    _legendaryActions(),
    _specialAbilities(),
    _reactions(),
    _batchChanges(false),
    _changesMade(false),
    _iconChanged(false),
    _scaledPixmaps()
{
    inputXML(element, isImport);
}

void MonsterClass::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    beginBatchChanges();

    setPrivate(static_cast<bool>(element.attribute("private",QString::number(0)).toInt()));
    setName(element.firstChildElement(QString("name")).text());
    setMonsterType(element.firstChildElement(QString("type")).text());
    setMonsterSubType(element.firstChildElement(QString("subtype")).text());
    setMonsterSize(element.firstChildElement(QString("size")).text());
    setSpeed(element.firstChildElement(QString("speed")).text());
    setAlignment(element.firstChildElement(QString("alignment")).text());
    setLanguages(element.firstChildElement(QString("languages")).text());
    setArmorClass(element.firstChildElement(QString("armor_class")).text().toInt());
    setAverageHitPoints(element.firstChildElement(QString("hit_points")).text().toInt());
    setHitDice(Dice(element.firstChildElement(QString("hit_dice")).text()));
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
        checkForSkill(element, SKILLELEMEMT_NAMES[s], static_cast<Combatant::Skills>(s), isImport);

    readActionList(element, QString("actions"), _actions, isImport);
    readActionList(element, QString("legendary_actions"), _legendaryActions, isImport);
    readActionList(element, QString("special_abilities"), _specialAbilities, isImport);
    readActionList(element, QString("reactions"), _reactions, isImport);

    readIcons(element, isImport);

    endBatchChanges();
}

QDomElement MonsterClass::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    element.setAttribute("private", static_cast<int>(getPrivate()));

    outputValue(doc, element, isExport, QString("name"), getName());
    outputValue(doc, element, isExport, QString("type"), getMonsterType());
    outputValue(doc, element, isExport, QString("subtype"), getMonsterSubType());
    outputValue(doc, element, isExport, QString("size"), getMonsterSize());
    outputValue(doc, element, isExport, QString("speed"), getSpeed());
    outputValue(doc, element, isExport, QString("alignment"), getAlignment());
    outputValue(doc, element, isExport, QString("languages"), getLanguages());
    outputValue(doc, element, isExport, QString("armor_class"), QString::number(getArmorClass()));
    outputValue(doc, element, isExport, QString("hit_dice"), getHitDice().toString());
    outputValue(doc, element, isExport, QString("hit_points"), QString::number(getAverageHitPoints()));
    outputValue(doc, element, isExport, QString("condition_immunities"), getConditionImmunities());
    outputValue(doc, element, isExport, QString("damage_immunities"), getDamageImmunities());
    outputValue(doc, element, isExport, QString("damage_resistances"), getDamageResistances());
    outputValue(doc, element, isExport, QString("damage_vulnerabilities"), getDamageVulnerabilities());
    outputValue(doc, element, isExport, QString("senses"), getSenses());
    outputValue(doc, element, isExport, QString("challenge_rating"), getChallenge());
    outputValue(doc, element, isExport, QString("strength"), QString::number(getStrength()));
    outputValue(doc, element, isExport, QString("dexterity"), QString::number(getDexterity()));
    outputValue(doc, element, isExport, QString("constitution"), QString::number(getConstitution()));
    outputValue(doc, element, isExport, QString("intelligence"), QString::number(getIntelligence()));
    outputValue(doc, element, isExport, QString("wisdom"), QString::number(getWisdom()));
    outputValue(doc, element, isExport, QString("charisma"), QString::number(getCharisma()));

    for(int s = Combatant::Skills_strengthSave; s < Combatant::SKILLS_COUNT; ++s)
    {
        if(_skillValues.contains(static_cast<Combatant::Skills>(s)))
            outputValue(doc, element, isExport, SKILLELEMEMT_NAMES[s], QString::number(_skillValues[s]));
    }

    writeIcons(doc, element, targetDirectory, isExport);

    writeActionList(doc, element, QString("actions"), _actions, isExport);
    writeActionList(doc, element, QString("legendary_actions"), _legendaryActions, isExport);
    writeActionList(doc, element, QString("special_abilities"), _specialAbilities, isExport);
    writeActionList(doc, element, QString("reactions"), _reactions, isExport);

    return element;
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
            emit iconChanged();

        if(_changesMade)
            emit dirty();
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

int MonsterClass::getIconCount() const
{
    return _icons.count();
}

QStringList MonsterClass::getIconList() const
{
    return _icons;
}

QString MonsterClass::getIcon(int index) const
{
    if((index < 0) || (index >= _icons.count()))
        return QString();
    else
        return _icons.at(index);
}

QPixmap MonsterClass::getIconPixmap(DMHelper::PixmapSize iconSize, int index)
{
    if((index < 0) || (index >= _scaledPixmaps.count()))
        return QPixmap();
    else
        return _scaledPixmaps[index].getPixmap(iconSize);
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

qreal MonsterClass::getMonsterSizeFactor() const
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
    return speedStr.leftRef(speedStr.indexOf(" ")).toInt();
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

int MonsterClass::getXP() const
{
    return getExperienceByCR(getChallenge());
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

QString MonsterClass::getSkillString() const
{
    QString result;

    QList<int> keyList = _skillValues.keys();
    for(int& s : keyList)
    {
        if(_skillValues.contains(s))
        {
            if(!result.isEmpty())
                result.append(", ");

            result.append(Character::getWrittenSkillName(s));
            result.append(" ");

            int v = _skillValues.value(s);
            if(v >= 0)
                result.append("+");

            result.append(QString::number(v));
        }
    }

    return result;
}

void MonsterClass::setSkillString(const QString& skills)
{
    if(skills.isEmpty())
        return;

    if(skills == getSkillString())
        return;

    QStringList skillList = skills.split(", ");
    if(skillList.count() <= 0)
        return;

    _skillValues.clear();

    for(QString& skillInfo : skillList)
    {
        int spaceIndex = skillInfo.lastIndexOf(" ");
        QString skillName = skillInfo.left(spaceIndex);
        QString skillValueString = skillInfo.right(skillInfo.length() - spaceIndex);
        bool convertSuccess = false;
        int skillValue = skillValueString.toUInt(&convertSuccess);
        if(convertSuccess)
        {
            int skillKey = Character::findKeyForSkillName(skillName);
            if(skillKey >= 0)
            {
                _skillValues[skillKey] = skillValue;
            }
        }
    }
}

bool MonsterClass::isSkillKnown(Combatant::Skills skill) const
{
    return _skillValues.contains(skill);
}

QList<MonsterAction> MonsterClass::getActions() const
{
    return _actions;
}

void MonsterClass::addAction(const MonsterAction& action)
{
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

void MonsterClass::cloneMonster(MonsterClass& other)
{
    beginBatchChanges();

    _private = other._private;
    _monsterType = other._monsterType;
    _monsterSubType = other._monsterSubType;
    _monsterSize = other._monsterSize;
    _monsterSizeCategory = other._monsterSizeCategory;
    _speed = other._speed;
    _alignment = other._alignment;
    _languages = other._languages;
    _armorClass = other._armorClass;
    _hitDice = other._hitDice;
    _averageHitPoints = other._averageHitPoints;
    _conditionImmunities = other._conditionImmunities;
    _damageImmunities = other._damageImmunities;
    _damageResistances = other._damageResistances;
    _damageVulnerabilities = other._damageVulnerabilities;
    _senses = other._senses;
    _challenge = other._challenge;

    _skillValues.clear(); // just in case we're cloning onto something that exists
    QList<int> otherKeys = other._skillValues.keys();
    for(int i = 0; i < otherKeys.count(); ++i)
    {
        int skillKey = otherKeys.at(i);
        _skillValues[skillKey] = other._skillValues.value(skillKey);
    }
//    for(int& skill : other._skillValues.keys())
//        _skillValues[skill] = other._skillValues.value(skill);

    _strength = other._strength;
    _dexterity = other._dexterity;
    _constitution = other._constitution;
    _intelligence = other._intelligence;
    _wisdom = other._wisdom;
    _charisma = other._charisma;

    _actions.clear();
    for(MonsterAction& action : other._actions)
        addAction(action);
    _legendaryActions.clear();
    for(MonsterAction& legendaryAction : other._legendaryActions)
        addLegendaryAction(legendaryAction);
    _specialAbilities.clear();
    for(MonsterAction& specialAbility : other._specialAbilities)
        addSpecialAbility(specialAbility);
    _reactions.clear();
    for(MonsterAction& reaction : other._reactions)
        addReaction(reaction);

    endBatchChanges();
}

int MonsterClass::convertSizeToCategory(const QString& monsterSize)
{
    if(QString::compare(monsterSize, QString("Tiny"),Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Tiny;
    else if(QString::compare(monsterSize, QString("Small"),Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Small;
    else if(QString::compare(monsterSize, QString("Medium"),Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Medium;
    else if(QString::compare(monsterSize, QString("Large"),Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Large;
    else if(QString::compare(monsterSize, QString("Huge"),Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Huge;
    else if(QString::compare(monsterSize, QString("Gargantuan"),Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Gargantuan;
    else if(QString::compare(monsterSize, QString("Colossal"),Qt::CaseInsensitive) == 0)
        return DMHelper::CombatantSize_Colossal;
    else
        return DMHelper::CombatantSize_Unknown;
}

QString MonsterClass::convertCategoryToSize(int category)
{
    switch(category)
    {
        case DMHelper::CombatantSize_Tiny:
            return QString("Tiny");
        case DMHelper::CombatantSize_Small:
            return QString("Small");
        case DMHelper::CombatantSize_Medium:
            return QString("Medium");
        case DMHelper::CombatantSize_Large:
            return QString("Large");
        case DMHelper::CombatantSize_Huge:
            return QString("Huge");
        case DMHelper::CombatantSize_Gargantuan:
            return QString("Gargantuan");
        case DMHelper::CombatantSize_Colossal:
            return QString("Colossal");
        default:
            return QString("Medium");
    }
}

qreal MonsterClass::convertSizeCategoryToScaleFactor(int category)
{
    switch(category)
    {
        case DMHelper::CombatantSize_Tiny:
            return 0.5;
        case DMHelper::CombatantSize_Small:
            return 0.75;
        case DMHelper::CombatantSize_Medium:
            return 1.0;
        case DMHelper::CombatantSize_Large:
            return 2.0;
        case DMHelper::CombatantSize_Huge:
            return 3.0;
        case DMHelper::CombatantSize_Gargantuan:
            return 4.0;
        case DMHelper::CombatantSize_Colossal:
            return 8.0;
        default:
            return 1.0;
    }
}

qreal MonsterClass::convertSizeToScaleFactor(const QString& monsterSize)
{
    return convertSizeCategoryToScaleFactor(convertSizeToCategory(monsterSize));
}

void MonsterClass::outputValue(QDomDocument &doc, QDomElement &element, bool isExport, const QString& valueName, const QString& valueText)
{
    Q_UNUSED(isExport);

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

void MonsterClass::addIcon(const QString &newIcon)
{
    if((newIcon.isEmpty()) || (_icons.contains(newIcon)))
        return;

//    QString searchResult = Bestiary::Instance()->findMonsterImage(getName(), newIcon);
//    if((searchResult.isEmpty()) || (_icons.contains(searchResult)))
//        return;

    _icons.append(newIcon);
    ScaledPixmap newPixmap;
    newPixmap.setBasePixmap(Bestiary::Instance()->getDirectory().filePath(newIcon));
    _scaledPixmaps.append(newPixmap);
    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged();
}

void MonsterClass::setIcon(int index, const QString& iconFile)
{
    if((index < 0) || (index >= _icons.count()))
        return;

    QString searchResult = Bestiary::Instance()->findMonsterImage(getName(), iconFile);
    if(searchResult.isEmpty())
        return;

    _icons[index] = searchResult;
    ScaledPixmap newPixmap;
    newPixmap.setBasePixmap(Bestiary::Instance()->getDirectory().filePath(searchResult));
    _scaledPixmaps[index] = newPixmap;
    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged();
}

void MonsterClass::removeIcon(int index)
{
    if((index < 0) || (index >= _icons.count()))
        return;

    _icons.removeAt(index);
    _scaledPixmaps.removeAt(index);
}

void MonsterClass::removeIcon(const QString& iconFile)
{
    removeIcon(_icons.indexOf(iconFile));
}

void MonsterClass::searchForIcons()
{
    QStringList searchResult = Bestiary::Instance()->findMonsterImages(getName());
    for(int i = 0; i < searchResult.count(); ++i)
        addIcon(searchResult.at(i));
}

/*
void MonsterClass::searchForIcon(const QString &newIcon)
{
    QString searchResult = Bestiary::Instance()->findMonsterImage(getName(), newIcon);
    if((!searchResult.isEmpty()) && (!_icons.contains(searchResult)))
    {
        _icons.append(searchResult);
        ScaledPixmap newPixmap;
        newPixmap.setBasePixmap(Bestiary::Instance()->getDirectory().filePath(searchResult));
        _scaledPixmaps.append(newPixmap);
        registerChange();

        if(_batchChanges)
            _iconChanged = true;
        else
            emit iconChanged();
    }
}
*/

void MonsterClass::refreshIconPixmaps()
{
    _scaledPixmaps.clear();
    for(int i = 0; i < _icons.count(); ++i)
    {
        ScaledPixmap newPixmap;
        newPixmap.setBasePixmap(Bestiary::Instance()->getDirectory().filePath(_icons.at(i)));
        _scaledPixmaps.append(newPixmap);
    }

    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged();
}

void MonsterClass::clearIcon()
{
    _icons.clear();
    _scaledPixmaps.clear();
    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged();
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
    setAverageHitPoints(_hitDice.average());
    registerChange();
}

void MonsterClass::setAverageHitPoints(int averageHitPoints)
{
    if(averageHitPoints == _averageHitPoints)
        return;

    _averageHitPoints = averageHitPoints;
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
        _changesMade = true;
    else
        emit dirty();
}

void MonsterClass::checkForSkill(const QDomElement& element, const QString& skillName, Combatant::Skills skill, bool isImport)
{
    Q_UNUSED(isImport);

    QDomElement skillElement = element.firstChildElement(skillName);
    if(skillElement.isNull())
        return;

    _skillValues[skill] = skillElement.text().toInt();
}

void MonsterClass::readActionList(const QDomElement& element, const QString& actionName, QList<MonsterAction>& actionList, bool isImport)
{
    QDomElement actionListElement = element.firstChildElement(actionName);
    if(actionListElement.isNull())
        return;

    QDomElement actionElement = actionListElement.firstChildElement("element");
    while(!actionElement.isNull())
    {
        MonsterAction newAction(actionElement, isImport);
        actionList.append(newAction);
        actionElement = actionElement.nextSiblingElement("element");
    }
}

void MonsterClass::writeActionList(QDomDocument &doc, QDomElement& element, const QString& actionName, const QList<MonsterAction>& actionList, bool isExport) const
{
    QDomElement actionListElement = doc.createElement(actionName);

    for(int i = 0; i < actionList.count(); ++i)
    {
        QDomElement actionElement = doc.createElement("element");
        actionList.at(i).outputXML(doc, actionElement, isExport);
        actionListElement.appendChild(actionElement);
    }

    element.appendChild(actionListElement);
}

void MonsterClass::readIcons(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.hasAttribute("icon"))
        addIcon(element.attribute("icon"));

    QDomElement iconElement = element.firstChildElement("icon");
    while(!iconElement.isNull())
    {
        addIcon(iconElement.attribute("filename"));
        iconElement = iconElement.nextSiblingElement("icon");
    }
}

void MonsterClass::writeIcons(QDomDocument &doc, QDomElement& element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(isExport);

    if(_icons.count() == 1)
    {
        if(!_icons.at(0).isEmpty())
            element.setAttribute("icon", targetDirectory.relativeFilePath(_icons.at(0)));
    }
    else
    {
        for(int i = 0; i < _icons.count(); ++i)
        {
            if(!_icons.at(i).isEmpty())
            {
                QDomElement iconElement = doc.createElement("icon");
                iconElement.setAttribute("filename", _icons.at(i));
                element.appendChild(iconElement);
            }
        }
    }
}

int MonsterClass::getExperienceByCR(const QString& inputCR)
{
    if(inputCR == QString("0")) return 0;
    if((inputCR == QString("0.125")) || (inputCR == QString("1/8"))) return 25;
    if((inputCR == QString("0.25")) || (inputCR == QString("1/4"))) return 50;
    if((inputCR == QString("0.5")) || (inputCR == QString("1/2"))) return 100;
    if(inputCR == QString("1")) return 200;
    if(inputCR == QString("2")) return 450;
    if(inputCR == QString("3")) return 700;
    if(inputCR == QString("4")) return 1100;
    if(inputCR == QString("5")) return 1800;
    if(inputCR == QString("6")) return 2300;
    if(inputCR == QString("7")) return 2900;
    if(inputCR == QString("8")) return 3900;
    if(inputCR == QString("9")) return 5000;
    if(inputCR == QString("10")) return 5900;
    if(inputCR == QString("11")) return 7200;
    if(inputCR == QString("12")) return 8400;
    if(inputCR == QString("13")) return 10000;
    if(inputCR == QString("14")) return 11500;
    if(inputCR == QString("15")) return 13000;
    if(inputCR == QString("16")) return 15000;
    if(inputCR == QString("17")) return 18000;
    if(inputCR == QString("18")) return 20000;
    if(inputCR == QString("19")) return 22000;
    if(inputCR == QString("20")) return 25000;
    if(inputCR == QString("21")) return 33000;
    if(inputCR == QString("22")) return 41000;
    if(inputCR == QString("23")) return 50000;
    if(inputCR == QString("24")) return 62000;
    if(inputCR == QString("25")) return 75000;
    if(inputCR == QString("26")) return 90000;
    if(inputCR == QString("27")) return 105000;
    if(inputCR == QString("28")) return 120000;
    if(inputCR == QString("29")) return 135000;
    if(inputCR == QString("30")) return 155000;

    return -1;
}

int MonsterClass::getProficiencyByCR(const QString& inputCR)
{
    if(inputCR == QString("0")) return 2;
    if((inputCR == QString("0.125")) || (inputCR == QString("1/8"))) return 2;
    if((inputCR == QString("0.25")) || (inputCR == QString("1/4"))) return 2;
    if((inputCR == QString("0.5")) || (inputCR == QString("1/2"))) return 2;
    if(inputCR == QString("1")) return 2;
    if(inputCR == QString("2")) return 2;
    if(inputCR == QString("3")) return 2;
    if(inputCR == QString("4")) return 2;
    if(inputCR == QString("5")) return 3;
    if(inputCR == QString("6")) return 3;
    if(inputCR == QString("7")) return 3;
    if(inputCR == QString("8")) return 3;
    if(inputCR == QString("9")) return 4;
    if(inputCR == QString("10")) return 4;
    if(inputCR == QString("11")) return 4;
    if(inputCR == QString("12")) return 4;
    if(inputCR == QString("13")) return 5;
    if(inputCR == QString("14")) return 5;
    if(inputCR == QString("15")) return 5;
    if(inputCR == QString("16")) return 5;
    if(inputCR == QString("17")) return 6;
    if(inputCR == QString("18")) return 6;
    if(inputCR == QString("19")) return 6;
    if(inputCR == QString("20")) return 6;
    if(inputCR == QString("21")) return 7;
    if(inputCR == QString("22")) return 7;
    if(inputCR == QString("23")) return 7;
    if(inputCR == QString("24")) return 7;
    if(inputCR == QString("25")) return 8;
    if(inputCR == QString("26")) return 8;
    if(inputCR == QString("27")) return 8;
    if(inputCR == QString("28")) return 8;
    if(inputCR == QString("29")) return 9;
    if(inputCR == QString("30")) return 9;

    return 0;
}

