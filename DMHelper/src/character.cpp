#include "character.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDir>
#include <QtDebug>

const char* STRINGVALUE_DEFAULTS[Character::STRINGVALUE_COUNT] =
{
    "N/A", // StringValue_player
    "", // StringValue_race
    "N/A", // StringValue_subrace
    "", // StringValue_sex
    "", // StringValue_alignment
    "N/A", // StringValue_background
    "", // StringValue_class
    "N/A", // StringValue_class2
    "N/A", // StringValue_class3
    "", // StringValue_age
    "", // StringValue_height
    "", // StringValue_weight
    "", // StringValue_eyes
    "", // StringValue_hair
    "", // StringValue_equipment
    "", // StringValue_proficiencies
    "", // StringValue_spells
    "",  // StringValue_notes
    "medium"  // StringValue_size
};

const char* STRINGVALUE_NAMES[Character::STRINGVALUE_COUNT] =
{
    "player", // StringValue_player
    "race", // StringValue_race
    "subrace", // StringValue_subrace
    "sex", // StringValue_sex
    "alignment", // StringValue_alignment
    "background", // StringValue_background
    "class", // StringValue_class
    "class2", // StringValue_class2
    "class3", // StringValue_class3
    "age", // StringValue_age
    "height", // StringValue_height
    "weight", // StringValue_weight
    "eyes", // StringValue_eyes
    "hair", // StringValue_hair
    "equipment", // StringValue_equipment
    "proficiencies", // StringValue_proficiencies
    "spells", // StringValue_spells
    "notes",  // StringValue_notes
    "size"  // StringValue_size
};

const int INTVALUE_DEFAULTS[Character::INTVALUE_COUNT] =
{
    1, // IntValue_level
    0, // IntValue_level2
    0, // IntValue_level3
    10, // IntValue_strength
    10, // IntValue_dexterity
    10, // IntValue_constitution
    10, // IntValue_intelligence
    10, // IntValue_wisdom
    10, // IntValue_charisma
    30, // IntValue_speed
    0, // IntValue_platinum
    0, // IntValue_gold
    0, // IntValue_silver
    0, // IntValue_copper
    0  // IntValue_experience
};

const char* INTVALUE_NAMES[Character::INTVALUE_COUNT] =
{
    "level", // IntValue_level
    "level2", // IntValue_level2
    "level3", // IntValue_level3
    "strength", // IntValue_strength
    "dexterity", // IntValue_dexterity
    "constitution", // IntValue_constitution
    "intelligence", // IntValue_intelligence
    "wisdom", // IntValue_wisdom
    "charisma", // IntValue_charisma
    "speed", // IntValue_speed
    "platinum", // IntValue_platinum
    "gold", // IntValue_gold
    "silver", // IntValue_silver
    "copper", // IntValue_copper
    "experience" // IntValue_experience
};

/*
const bool BOOLVALUE_DEFAULTS[Character::BOOLVALUE_COUNT] =
{
    false, // BoolValue_strengthSave
    false, // BoolValue_athletics
    false, // BoolValue_dexteritySave
    false, // BoolValue_stealth
    false, // BoolValue_acrobatics
    false, // BoolValue_sleightOfHand
    false, // BoolValue_constitutionSave
    false, // BoolValue_intelligenceSave
    false, // BoolValue_investigation
    false, // BoolValue_arcana
    false, // BoolValue_nature
    false, // BoolValue_history
    false, // BoolValue_religion
    false, // BoolValue_wisdomSave
    false, // BoolValue_medicine
    false, // BoolValue_animalHandling
    false, // BoolValue_perception
    false, // BoolValue_insight
    false, // BoolValue_survival
    false, // BoolValue_charismaSave
    false, // BoolValue_performance
    false, // BoolValue_deception
    false, // BoolValue_persuasion
    false // BoolValue_intimidation
//    true // BoolValue_active
};
*/

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
//    "active"            // BoolValue_active
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
//    "Active"            // BoolValue_active
};

Character::Character(QObject *parent) :
    Combatant(parent),
    _dndBeyondID(-1),
    _stringValues(STRINGVALUE_COUNT),
    _intValues(INTVALUE_COUNT),
    _skillValues(SKILLS_COUNT),
    _active(true),
    _iconChanged(false)
{
    // Set default 1 HP to avoid confusion with characters not being displayed in the battles
    setHitPoints(1);

    setDefaultValues();
}

Character::Character(QDomElement &element, QObject *parent) :
    Combatant(parent),
    _dndBeyondID(-1),
    _stringValues(STRINGVALUE_COUNT),
    _intValues(INTVALUE_COUNT),
    _skillValues(SKILLS_COUNT),
    _active(true),
    _iconChanged(false)
{
    inputXML(element);
}

Character::Character(const Character &obj) :
    Combatant(obj),
    _dndBeyondID(obj._dndBeyondID),
    _stringValues(obj._stringValues),
    _intValues(obj._intValues),
    _skillValues(obj._skillValues),
    _active(true),
    _iconChanged(obj._iconChanged)
{
    qDebug("[Character] WARNING: Character copied - this is a highly questionable action!");
}

void Character::inputXML(const QDomElement &element)
{
    beginBatchChanges();

    Combatant::inputXML(element);

    setDndBeyondID(element.attribute(QString("dndBeyondID"),QString::number(-1)).toInt());

    int i;
    for(i = 0; i < STRINGVALUE_COUNT; ++i)
    {
        setStringValue((StringValue)i, element.attribute(STRINGVALUE_NAMES[i],STRINGVALUE_DEFAULTS[i]) );
    }

    for(i = 0; i < INTVALUE_COUNT; ++i)
    {
        setIntValue((IntValue)i, element.attribute(INTVALUE_NAMES[i],QString::number(INTVALUE_DEFAULTS[i])).toInt() );
    }

    for(i = 0; i < SKILLS_COUNT; ++i)
    {
        //setBoolValue((BoolValue)i, (bool)element.attribute(BOOLVALUE_NAMES[i],QString::number((int)BOOLVALUE_DEFAULTS[i])).toInt() );
        setSkillValue((Skills)i, (bool)element.attribute(SKILLVALUE_NAMES[i],QString::number((int)false)).toInt() );
    }

    setActive((bool)element.attribute(QString("active"),QString::number(true)).toInt());

    endBatchChanges();
}

void Character::beginBatchChanges()
{
    _iconChanged = false;

    Combatant::beginBatchChanges();
}

void Character::endBatchChanges()
{
    if(_batchChanges)
    {
        if(_iconChanged)
        {
            emit iconChanged();
        }
    }

    Combatant::endBatchChanges();
}

Combatant* Character::clone() const
{
    qDebug("[Character] WARNING: Character cloned - this is a highly questionable action!");
    return new Character(*this);
}

int Character::getType() const
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

void Character::setIcon(const QString &newIcon)
{
    if(newIcon != _icon)
    {
        _icon = newIcon;
        _iconPixmap.setBasePixmap(_icon);
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
    if((key < 0)||(key >= STRINGVALUE_COUNT))
    {
        qWarning() << "[Character] Illegal string value requested from character. Id: " << key;
        return QString();
    }

    return _stringValues[key];
}

int Character::getIntValue(IntValue key) const
{
    if((key < 0)||(key >= INTVALUE_COUNT))
    {
        qWarning() << "[Character] Illegal int value requested from character. Id: " << key;
        return 0;
    }

    return _intValues[key];
}

bool Character::getSkillValue(Skills key) const
{
    if((key < 0)||(key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Illegal skill value requested from character. Id: " << key;
        return false;
    }

    return _skillValues[key];
}

void Character::setStringValue(StringValue key, const QString& value)
{
    if((key < 0)||(key >= STRINGVALUE_COUNT))
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
    if((key < 0)||(key >= INTVALUE_COUNT))
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
    if((key < 0)||(key >= SKILLS_COUNT))
    {
        qWarning() << "[Character] Tried to set illegal skill value from character. Id: " << key;
        return;
    }

    if(_skillValues[key] != value)
    {
        _skillValues[key] = value;
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
    // return getIntValue(IntValue_level) + getIntValue(IntValue_level2) + getIntValue(IntValue_level3);
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
    int result = 10;

    result += Combatant::getAbilityMod(getIntValue(IntValue_wisdom));

    if(getSkillValue(Skills_perception))
    {
        result += getProficiencyBonus();
    }

    return result;
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

void Character::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);

    element.setAttribute( "dndBeyondID", getDndBeyondID() );

    int i;
    for(i = 0; i < STRINGVALUE_COUNT; ++i)
    {
        element.setAttribute( STRINGVALUE_NAMES[i], getStringValue((StringValue)i) );
    }

    for(i = 0; i < INTVALUE_COUNT; ++i)
    {
        element.setAttribute( INTVALUE_NAMES[i], getIntValue((IntValue)i) );
    }

    for(i = 0; i < SKILLS_COUNT; ++i)
    {
        element.setAttribute( SKILLVALUE_NAMES[i], (int)getSkillValue((Skills)i) );
    }

    element.setAttribute("active", (int)getActive());
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
        //_boolValues[i] = BOOLVALUE_DEFAULTS[i];
        _skillValues[i] = false;
    }

    _active = true;
}
