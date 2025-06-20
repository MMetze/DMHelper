#include "character.h"
#include "dmconstants.h"
#include "monsterclass.h"
#include "bestiary.h"
#include <QDomElement>
#include <QDir>
#include <QIcon>
#include <QDebug>

const char* STRINGVALUE_DEFAULTS[Character::STRINGVALUE_COUNT] =
{
    "N/A",      // StringValue_player
    "",         // StringValue_race
    "N/A",      // StringValue_subrace
    "",         // StringValue_sex
    "",         // StringValue_alignment
    "N/A",      // StringValue_background
    "",         // StringValue_class
    "N/A",      // StringValue_class2
    "N/A",      // StringValue_class3
    "",         // StringValue_age
    "",         // StringValue_height
    "",         // StringValue_weight
    "",         // StringValue_eyes
    "",         // StringValue_hair
    "",         // StringValue_equipment
    "",         // StringValue_proficiencies
    "",         // StringValue_notes
    "medium",   // StringValue_size
    "0"         // StringValue_experience
};

const char* STRINGVALUE_NAMES[Character::STRINGVALUE_COUNT] =
{
    "player",           // StringValue_player
    "race",             // StringValue_race
    "subrace",          // StringValue_subrace
    "sex",              // StringValue_sex
    "alignment",        // StringValue_alignment
    "background",       // StringValue_background
    "class",            // StringValue_class
    "class2",           // StringValue_class2
    "class3",           // StringValue_class3
    "age",              // StringValue_age
    "height",           // StringValue_height
    "weight",           // StringValue_weight
    "eyes",             // StringValue_eyes
    "hair",             // StringValue_hair
    "equipment",        // StringValue_equipment
    "proficiencies",    // StringValue_proficiencies
    "notes",            // StringValue_notes
    "size",             // StringValue_size
    "experience"        // StringValue_experience
};

const int INTVALUE_DEFAULTS[Character::INTVALUE_COUNT] =
{
    1,      // IntValue_level
    0,      // IntValue_level2
    0,      // IntValue_level3
    10,     // IntValue_strength
    10,     // IntValue_dexterity
    10,     // IntValue_constitution
    10,     // IntValue_intelligence
    10,     // IntValue_wisdom
    10,     // IntValue_charisma
    30,     // IntValue_speed
    0,      // IntValue_platinum
    0,      // IntValue_gold
    0,      // IntValue_silver
    0,      // IntValue_copper
    0,      // IntValue_jackofalltrades
    1,      // IntValue_maximumHP
    0,      // IntValue_pactMagicSlots
    0,      // IntValue_pactMagicUsed
    0,      // IntValue_pactMagicLevel
    0,      // IntValue_cantrips
};

const char* INTVALUE_NAMES[Character::INTVALUE_COUNT] =
{
    "level",            // IntValue_level
    "level2",           // IntValue_level2
    "level3",           // IntValue_level3
    "strength",         // IntValue_strength
    "dexterity",        // IntValue_dexterity
    "constitution",     // IntValue_constitution
    "intelligence",     // IntValue_intelligence
    "wisdom",           // IntValue_wisdom
    "charisma",         // IntValue_charisma
    "speed",            // IntValue_speed
    "platinum",         // IntValue_platinum
    "gold",             // IntValue_gold
    "silver",           // IntValue_silver
    "copper",           // IntValue_copper
    "jackofalltrades",  // IntValue_jackofalltrades
    "maximumhp",        // IntValue_maximumHP
    "pactmagicslots",   // IntValue_pactMagicSlots
    "pactmagicused",    // IntValue_pactMagicUsed
    "pactmagiclevel",   // IntValue_pactMagicLevel
    "cantrips",         // IntValue_cantrips
};

const char* SKILLVALUE_NAMES[Combatant::SKILLS_COUNT] =
{
    "strengthSave",     // Skills_strengthSave
    "athletics",        // Skills_athletics
    "dexteritySave",    // Skills_dexteritySave
    "stealth",          // Skills_stealth
    "acrobatics",       // Skills_acrobatics
    "sleightOfHand",    // Skills_sleightOfHand
    "constitutionSave", // Skills_constitutionSave
    "intelligenceSave", // Skills_intelligenceSave
    "investigation",    // Skills_investigation
    "arcana",           // Skills_arcana
    "nature",           // Skills_nature
    "history",          // Skills_history
    "religion",         // Skills_religion
    "wisdomSave",       // Skills_wisdomSave
    "medicine",         // Skills_medicine
    "animalHandling",   // Skills_animalHandling
    "perception",       // Skills_perception
    "insight",          // Skills_insight
    "survival",         // Skills_survival
    "charismaSave",     // Skills_charismaSave
    "performance",      // Skills_performance
    "deception",        // Skills_deception
    "persuasion",       // Skills_persuasion
    "intimidation"      // Skills_intimidation
};

const char* SKILLVALUE_WRITTENNAMES[Combatant::SKILLS_COUNT] =
{
    "Str",              // Skills_strengthSave
    "Athletics",        // Skills_athletics
    "Dex",              // Skills_dexteritySave
    "Stealth",          // Skills_stealth
    "Acrobatics",       // Skills_acrobatics
    "Sleight of Hand",  // Skills_sleightOfHand
    "Con",              // Skills_constitutionSave
    "Int",              // Skills_intelligenceSave
    "Investigation",    // Skills_investigation
    "Arcana",           // Skills_arcana
    "Nature",           // Skills_nature
    "History",          // Skills_history
    "Religion",         // Skills_religion
    "Wis",              // Skills_wisdomSave
    "Medicine",         // Skills_medicine
    "Animal Handling",  // Skills_animalHandling
    "Perception",       // Skills_perception
    "Insight",          // Skills_insight
    "Survival",         // Skills_survival
    "Cha",              // Skills_charismaSave
    "Performance",      // Skills_performance
    "Deception",        // Skills_deception
    "Persuasion",       // Skills_persuasion
    "Intimidation"      // Skills_intimidation
};

Character::Character(const QString& name, QObject *parent) :
    Combatant(name, parent),
    _dndBeyondID(-1),
    _stringValues(STRINGVALUE_COUNT),
    _intValues(INTVALUE_COUNT),
    _skillValues(SKILLS_COUNT),
    _spellSlots(),
    _spellSlotsUsed(),
    _spellList(),
    _active(true),
    _iconChanged(false)
{
    // Set default 1 HP to avoid confusion with characters not being displayed in the battles
    setHitPoints(1);

    setDefaultValues();
}

void Character::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();

    setDndBeyondID(element.attribute(QString("dndBeyondID"), QString::number(-1)).toInt());

    int i;
    for(i = 0; i < STRINGVALUE_COUNT; ++i)
    {
        setStringValue(static_cast<StringValue>(i), element.attribute(STRINGVALUE_NAMES[i], STRINGVALUE_DEFAULTS[i]));
    }

    for(i = 0; i < INTVALUE_COUNT; ++i)
    {
        setIntValue(static_cast<IntValue>(i), element.attribute(INTVALUE_NAMES[i], QString::number(INTVALUE_DEFAULTS[i])).toInt());
    }

    for(i = 0; i < SKILLS_COUNT; ++i)
    {
        setSkillValue(static_cast<Skills>(i), element.attribute(SKILLVALUE_NAMES[i], QString::number(0)).toInt());
    }

    i = 0;
    while(element.hasAttribute(QString("slots") + QString::number(i+1)))
    {
        setSpellSlots(i+1, element.attribute(QString("slots") + QString::number(i+1)).toInt());
        setSpellSlotsUsed(i+1, element.attribute(QString("slotsused") + QString::number(i+1)).toInt());
        ++i;
    }

    if(element.hasAttribute(QString("spells")))
    {
        _spellList = element.attribute("spells"); // backwards compatibility
    }
    else
    {
        QDomElement spellElement = element.firstChildElement("spell-data");
        if(!spellElement.isNull())
        {
            QDomNode spellDataChildNode = spellElement.firstChild();
            if((!spellDataChildNode.isNull()) && (spellDataChildNode.isCDATASection()))
            {
                QDomCDATASection spellData = spellDataChildNode.toCDATASection();
                _spellList = spellData.data();
            }
        }
    }

    setActive(static_cast<bool>(element.attribute(QString("active"), QString::number(true)).toInt()));

    Combatant::inputXML(element, isImport);

    if(!element.hasAttribute(INTVALUE_NAMES[IntValue_maximumHP]))
    {
        setIntValue(IntValue_maximumHP, getHitPoints());
    }

    readActionList(element, QString("actions"), _actions, isImport);

    endBatchChanges();
}

void Character::copyValues(const CampaignObjectBase* other)
{
    const Character* otherCharacter = dynamic_cast<const Character*>(other);
    if(!otherCharacter)
        return;

    _dndBeyondID = otherCharacter->_dndBeyondID;
    _stringValues = otherCharacter->_stringValues;
    _intValues = otherCharacter->_intValues;
    _skillValues = otherCharacter->_skillValues;
    _spellSlots = otherCharacter->_spellSlots;
    _spellSlotsUsed = otherCharacter->_spellSlotsUsed;
    _spellList = otherCharacter->_spellList;
    _active = otherCharacter->_active;

    _actions.clear();
    for(const MonsterAction& action : otherCharacter->_actions)
        addAction(action);

    Combatant::copyValues(other);
}

QIcon Character::getDefaultIcon()
{
    if(_iconPixmap.isValid())
        return QIcon(_iconPixmap.getPixmap(DMHelper::PixmapSize_Battle).scaled(128,128, Qt::KeepAspectRatio));
    else
        return isInParty() ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png");
}

void Character::beginBatchChanges()
{
    _iconChanged = false;

    Combatant::beginBatchChanges();
}

void Character::endBatchChanges()
{
    if((_batchChanges) && (_iconChanged))
        emit iconChanged(this);

    Combatant::endBatchChanges();
}

Combatant* Character::clone() const
{
    qDebug() << "[Character] WARNING: Character cloned - this is a highly questionable action!";

    Character* newCharacter = new Character(getName());
    newCharacter->copyValues(this);

    return newCharacter;
}

int Character::getCombatantType() const
{
    return DMHelper::CombatantType_Character;
}

int Character::getDndBeyondID() const
{
    return _dndBeyondID;
}

void Character::setDndBeyondID(int id)
{
    _dndBeyondID = id;
}

bool Character::isInParty() const
{
    return (getParentByType(DMHelper::CampaignType_Party) != nullptr);
}

void Character::setIcon(const QString &newIcon)
{
    if(newIcon != _icon)
    {
        _icon = newIcon;
        _iconPixmap.setBasePixmap(_icon);
        registerChange();

        if(_batchChanges)
            _iconChanged = true;
        else
            emit iconChanged(this);
    }
}

int Character::getSpeed() const
{
    return getIntValue(IntValue_speed);
}

int Character::getStrength() const
{
    return getIntValue(IntValue_strength);
}

int Character::getDexterity() const
{
    return getIntValue(IntValue_dexterity);
}

int Character::getConstitution() const
{
    return getIntValue(IntValue_constitution);
}

int Character::getIntelligence() const
{
    return getIntValue(IntValue_intelligence);
}

int Character::getWisdom() const
{
    return getIntValue(IntValue_wisdom);
}

int Character::getCharisma() const
{
    return getIntValue(IntValue_charisma);
}

QString Character::getStringValue(StringValue key) const
{
    if((key < 0) || (key >= STRINGVALUE_COUNT))
    {
        qWarning() << "[Character] Illegal string value requested from character. Id: " << key;
        return QString();
    }

    return _stringValues[key];
}

int Character::getIntValue(IntValue key) const
{
    if((key < 0) || (key >= INTVALUE_COUNT))
    {
        qWarning() << "[Character] Illegal int value requested from character. Id: " << key;
        return 0;
    }

    return _intValues[key];
}

bool Character::getSkillValue(Skills key) const
{
    if((key < 0) || (key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Illegal skill value requested from character. Id: " << key;
        return false;
    }

    return (_skillValues[key] > 0);
}

bool Character::getSkillExpertise(Skills key) const
{
    if((key < 0) || (key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Illegal skill expertise value requested from character. Id: " << key;
        return false;
    }

    return (_skillValues[key] > 1);
}

int Character::getSkillBonus(Skills key) const
{
    if((key < 0) && (key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Illegal skill bonus requested from character. Id: " << key;
        return 0;
    }

    int skillBonus = getAbilityMod(getAbilityValue(getSkillAbility(key)));
    if(getSkillExpertise(key))
        skillBonus += getProficiencyBonus() * 2;
    else if(getSkillValue(key))
        skillBonus += getProficiencyBonus();
    else if((getIntValue(Character::IntValue_jackofalltrades) > 0) &&
            (!isSkillSavingThrow(key)))
        skillBonus += getProficiencyBonus() / 2;

    return skillBonus;
}

void Character::setStringValue(StringValue key, const QString& value)
{
    if((key < 0) || (key >= STRINGVALUE_COUNT))
    {
        qWarning() << "[Character] Tried to set illegal string value from character. Id: " << key;
        return;
    }

    if(_stringValues[key] != value)
    {
        _stringValues[key] = value;
        registerChange();
    }
}

void Character::setIntValue(IntValue key, int value)
{
    if((key < 0) || (key >= INTVALUE_COUNT))
    {
        qWarning() << "[Character] Tried to set illegal int value from character. Id: " << key;
        return;
    }

    if(_intValues[key] != value)
    {
        _intValues[key] = value;
        registerChange();
    }
}

void Character::setSkillValue(Skills key, bool value)
{
    if((key < 0) || (key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Tried to set illegal skill value from character. Id: " << key;
        return;
    }

    if(((value) && (_skillValues[key] == Combatant::SKILLS_UNSKILLED)) ||
       ((!value) && (_skillValues[key] > Combatant::SKILLS_UNSKILLED)))
    {
        _skillValues[key] = (value ? Combatant::SKILLS_SKILLED : Combatant::SKILLS_UNSKILLED);
        registerChange();
    }
}

void Character::setSkillValue(Skills key, int value)
{
    if((key < 0) || (key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Tried to set illegal skill value from character. Id: " << key;
        return;
    }

    if((value != _skillValues[key]))
    {
        _skillValues[key] = value;
        registerChange();
    }
}

void Character::setSkillExpertise(Skills key, bool value)
{
    if((key < 0) || (key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Tried to set illegal skill value from character. Id: " << key;
        return;
    }

    if(_skillValues[key] == Combatant::SKILLS_UNSKILLED)
        return;

    if((value) && (_skillValues[key] == Combatant::SKILLS_SKILLED))
    {
        _skillValues[key] = Combatant::SKILLS_EXPERT;
        registerChange();
    }

    if((!value) && (_skillValues[key] == Combatant::SKILLS_EXPERT))
    {
        _skillValues[key] = Combatant::SKILLS_SKILLED;
        registerChange();
    }
}

int Character::spellSlotLevels() const
{
    return _spellSlots.size();
}

QVector<int> Character::getSpellSlots() const
{
    return _spellSlots;
}

QVector<int> Character::getSpellSlotsUsed() const
{
    return _spellSlotsUsed;
}

void Character::setSpellSlots(int level, int slotCount)
{
    if((level <= 0) || (slotCount < 0))
        return;

    while(level > _spellSlots.size())
    {
        _spellSlots.append(0);
        _spellSlotsUsed.append(0);
    }

    if(slotCount == 0)
    {
        while(level <= _spellSlots.size())
            _spellSlots.takeLast();

        _spellSlotsUsed.resize(_spellSlots.size());
    }
    else
    {
        _spellSlots[level - 1] = slotCount;
    }

    registerChange();
}

int Character::getSpellSlots(int level)
{
    return ((level <= 0) || (level > _spellSlots.size())) ? 0 : _spellSlots.at(level - 1);
}

void Character::setSpellSlotsUsed(int level, int slotsUsed)
{
    if((level <= 0) || (level > _spellSlotsUsed.size()) || (slotsUsed < 0))
        return;

    int newSlotsUsed = (slotsUsed > _spellSlots.at(level - 1)) ? _spellSlots.at(level - 1) : slotsUsed;
    if(_spellSlotsUsed[level - 1] == newSlotsUsed)
        return;

    _spellSlotsUsed[level - 1] = newSlotsUsed;
    registerChange();
}

int Character::getSpellSlotsUsed(int level)
{
    return ((level <= 0) || (level > _spellSlotsUsed.size())) ? 0 : _spellSlotsUsed.at(level - 1);
}

void Character::clearSpellSlotsUsed()
{
    _spellSlotsUsed.fill(0);
    registerChange();
}

QString Character::getSpellString()
{
    return _spellList;
}

void Character::setSpellString(const QString& spellString)
{
    if(_spellList != spellString)
    {
        _spellList = spellString;
        registerChange();
    }
}

bool Character::getActive() const
{
    return _active;
}

void Character::setActive(bool active)
{
    if(_active != active)
    {
        _active = active;
        registerChange();
    }
}

int Character::getTotalLevel() const
{
    // Only supporting a single level currently
    return getIntValue(IntValue_level);
}

int Character::getXPThreshold(int threshold) const
{
    // 0-based level
    return DMHelper::XPThresholds[threshold][getTotalLevel() - 1];
}

int Character::getNextLevelXP() const
{
    // Note 0-based levels, but we want the next level, so it's +1-1
    return DMHelper::XPProgression[getTotalLevel()];
}

int Character::getProficiencyBonus() const
{
    // 0-based level
    return DMHelper::ProficiencyProgression[getTotalLevel() - 1];
}

int Character::getPassivePerception() const
{
    return 10 + getSkillBonus(Skills_perception);
}

QList<MonsterAction> Character::getActions() const
{
    return _actions;
}

void Character::addAction(const MonsterAction& action)
{
    _actions.append(action);
    registerChange();
}

void Character::setAction(int index, const MonsterAction& action)
{
    if((index < 0) || (index >= _actions.count()))
        return;

    if(_actions.at(index) != action)
    {
        _actions[index] = action;
        registerChange();
    }
}

int Character::removeAction(const MonsterAction& action)
{
    _actions.removeAll(action);
    registerChange();
    return _actions.count();
}

void Character::copyMonsterValues(MonsterClass& monster)
{
    beginBatchChanges();

    if(Bestiary::Instance())
    {
        QString monsterImageFile = Bestiary::Instance()->findMonsterImage(monster.getName(), monster.getIcon());
        QDir bestiaryDir = Bestiary::Instance()->getDirectory();
        if((!monsterImageFile.isEmpty()) && (bestiaryDir.exists(monsterImageFile)))
            setIcon(bestiaryDir.filePath(monsterImageFile));
    }

    setStringValue(StringValue_race, monster.getName());
    setStringValue(StringValue_size, monster.getMonsterSize());
    setIntValue(IntValue_speed, monster.getSpeedValue());
    setStringValue(StringValue_alignment, monster.getAlignment());
    setArmorClass(monster.getArmorClass());
    setHitDice(monster.getHitDice());
    setHitPoints(monster.getAverageHitPoints());

    setIntValue(IntValue_strength, monster.getStrength());
    setIntValue(IntValue_dexterity, monster.getDexterity());
    setIntValue(IntValue_constitution, monster.getConstitution());
    setIntValue(IntValue_intelligence, monster.getIntelligence());
    setIntValue(IntValue_wisdom, monster.getWisdom());
    setIntValue(IntValue_charisma, monster.getCharisma());

    // Check skills
    for(int skillIt = 0; skillIt < SKILLS_COUNT; ++skillIt)
    {
        Skills skill = static_cast<Skills>(skillIt);
        if(monster.isSkillKnown(skill))
            setSkillValue(skill, monster.getSkillValue(skill));
    }

    // Proficiencies
    QString proficiencyString;
    if(!monster.getSenses().isEmpty())
        proficiencyString += QString("Senses:") + QChar::LineFeed + monster.getSenses() + QChar::LineFeed + QChar::LineFeed;
    if(!monster.getLanguages().isEmpty())
        proficiencyString += QString("Languages:") + QChar::LineFeed + monster.getLanguages() + QChar::LineFeed + QChar::LineFeed;
    if(!monster.getConditionImmunities().isEmpty())
        proficiencyString += QString("Condition Immunities:") + QChar::LineFeed + monster.getConditionImmunities() + QChar::LineFeed + QChar::LineFeed;
    if(!monster.getDamageImmunities().isEmpty())
        proficiencyString += QString("Damage Immunities:") + QChar::LineFeed + monster.getDamageImmunities() + QChar::LineFeed + QChar::LineFeed;
    if(!monster.getDamageResistances().isEmpty())
        proficiencyString += QString("Damage Resistances:") + QChar::LineFeed + monster.getDamageResistances() + QChar::LineFeed + QChar::LineFeed;
    if(!monster.getDamageVulnerabilities().isEmpty())
        proficiencyString += QString("Damage Vulnerabilities:") + QChar::LineFeed + monster.getDamageVulnerabilities() + QChar::LineFeed + QChar::LineFeed;
    setStringValue(StringValue_proficiencies, proficiencyString);

    // Notes and actions
    QString notesString;
    if(monster.getActions().count() > 0)
    {
        notesString += QString("Actions:") + QChar::LineFeed;
        for(MonsterAction monsterAction : monster.getActions())
        {
            notesString += monsterAction.summaryString() + QChar::LineFeed;
            addAction(monsterAction);
        }
        notesString += QChar::LineFeed;
    }
    if(monster.getLegendaryActions().count() > 0)
    {
        notesString += QString("Legendary Actions:") + QChar::LineFeed;
        for(MonsterAction legendaryAction : monster.getLegendaryActions())
            notesString += legendaryAction.summaryString() + QChar::LineFeed;
        notesString += QChar::LineFeed;
    }
    if(monster.getSpecialAbilities().count() > 0)
    {
        notesString += QString("Special Abilities:") + QChar::LineFeed;
        for(MonsterAction specialAbility : monster.getSpecialAbilities())
            notesString += specialAbility.summaryString() + QChar::LineFeed;
        notesString += QChar::LineFeed;
    }
    if(monster.getReactions().count() > 0)
    {
        notesString += QString("Reactions:") + QChar::LineFeed;
        for(MonsterAction monsterReaction : monster.getReactions())
            notesString += monsterReaction.summaryString() + QChar::LineFeed;
        notesString += QChar::LineFeed;
    }
    setStringValue(StringValue_notes, notesString);

    endBatchChanges();
}

int Character::findKeyForSkillName(const QString& skillName)
{
    for(int i = 0; i < SKILLS_COUNT; ++i)
    {
        if(skillName.compare(SKILLVALUE_WRITTENNAMES[i], Qt::CaseInsensitive) == 0)
            return i;
    }

    return -1;
}

QString Character::getWrittenSkillName(int skill)
{
    if((skill < 0) || (skill >= Combatant::SKILLS_COUNT))
    {
        return QString();
    }
    else
    {
        return QString(SKILLVALUE_WRITTENNAMES[skill]);
    }
}

void Character::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("dndBeyondID", getDndBeyondID());

    int i;
    for(i = 0; i < STRINGVALUE_COUNT; ++i)
    {
        element.setAttribute(STRINGVALUE_NAMES[i], getStringValue(static_cast<StringValue>(i)));
    }

    for(i = 0; i < INTVALUE_COUNT; ++i)
    {
        element.setAttribute(INTVALUE_NAMES[i], getIntValue(static_cast<IntValue>(i)));
    }

    for(i = 0; i < SKILLS_COUNT; ++i)
    {
        element.setAttribute(SKILLVALUE_NAMES[i], _skillValues[static_cast<Skills>(i)]);
    }

    for(i = 0; i < _spellSlots.size(); ++i)
    {
        element.setAttribute(QString("slots") + QString::number(i+1), _spellSlots.at(i));
        element.setAttribute(QString("slotsused") + QString::number(i+1), _spellSlotsUsed.at(i));
    }

    element.setAttribute("active", static_cast<int>(getActive()));

    if(!_spellList.isEmpty())
    {
        QDomElement spellElement = doc.createElement("spell-data");
            QDomCDATASection spellData = doc.createCDATASection(_spellList);
            spellElement.appendChild(spellData);
        element.appendChild(spellElement);
    }

    writeActionList(doc, element, QString("actions"), _actions, isExport);

    Combatant::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Character::belongsToObject(QDomElement& element)
{
    if((element.tagName() == QString("actions")) || (element.tagName() == QString("spell-data")))
        return true;
    else
        return Combatant::belongsToObject(element);
}

void Character::setDefaultValues()
{
    int i;

    for(i = 0; i < _stringValues.size(); ++i)
    {
        _stringValues[i] = STRINGVALUE_DEFAULTS[i];
    }

    for(i = 0; i < _intValues.size(); ++i)
    {
        _intValues[i] = INTVALUE_DEFAULTS[i];
    }

    for(i = 0; i < _skillValues.size(); ++i)
    {
        _skillValues[i] = 0;
    }

    _spellSlots.clear();
    _spellSlotsUsed.clear();

    _active = true;
}

void Character::readActionList(const QDomElement& element, const QString& actionName, QList<MonsterAction>& actionList, bool isImport)
{
    QDomElement actionListElement = element.firstChildElement(actionName);
    if(actionListElement.isNull())
        return;

    QDomElement actionElement = actionListElement.firstChildElement("action");
    while(!actionElement.isNull())
    {
        MonsterAction newAction(actionElement, isImport);
        actionList.append(newAction);
        actionElement = actionElement.nextSiblingElement("action");
    }
}

void Character::writeActionList(QDomDocument &doc, QDomElement& element, const QString& actionName, const QList<MonsterAction>& actionList, bool isExport) const
{
    QDomElement actionListElement = doc.createElement(actionName);

    for(int i = 0; i < actionList.count(); ++i)
    {
        QDomElement actionElement = doc.createElement("action");
        actionList.at(i).outputXML(doc, actionElement, isExport);
        actionListElement.appendChild(actionElement);
    }

    element.appendChild(actionListElement);
}
