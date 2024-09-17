#include "characterv2converter.h"
#include "character.h"
#include <QDomElement>

Characterv2Converter::Characterv2Converter() :
    Characterv2{}
{}

void Characterv2Converter::inputXML(const QDomElement &element, bool isImport)
{
    Character* convertCharacter = new Character();
    convertCharacter->inputXML(element, isImport);

    beginBatchChanges();

    setDndBeyondID(element.attribute(QString("dndBeyondID"), QString::number(-1)).toInt());
    convertValues(convertCharacter);

    Combatant::inputXML(element, isImport);

    endBatchChanges();

    delete convertCharacter;
}

void Characterv2Converter::convertValues(Character* convertCharacter)
{
    // Walk through the Character object and convert the values to the Characterv2 object
    setStringValue("dndBeyondID", QString::number(convertCharacter->getDndBeyondID()));
    setStringValue("size", convertCharacter->getStringValue(Character::StringValue_size));
    setIntValue("speed", convertCharacter->getIntValue(Character::IntValue_speed));

    setIntValue("strength", convertCharacter->getStrength());
    setIntValue("strengthMod", Combatant::getAbilityMod(convertCharacter->getStrength()));
    setIntValue("strengthSave", convertCharacter->getSkillBonus(Combatant::Skills_strengthSave));
    setIntValue("dexterity", convertCharacter->getDexterity());
    setIntValue("dexterityMod", Combatant::getAbilityMod(convertCharacter->getDexterity()));
    setIntValue("dexteritySave", convertCharacter->getSkillBonus(Combatant::Skills_dexteritySave));
    setIntValue("constitution", convertCharacter->getConstitution());
    setIntValue("constitutionMod", Combatant::getAbilityMod(convertCharacter->getConstitution()));
    setIntValue("constitutionSave", convertCharacter->getSkillBonus(Combatant::Skills_constitutionSave));
    setIntValue("intelligence", convertCharacter->getIntelligence());
    setIntValue("intelligenceMod", Combatant::getAbilityMod(convertCharacter->getIntelligence()));
    setIntValue("intelligenceSave", convertCharacter->getSkillBonus(Combatant::Skills_intelligenceSave));
    setIntValue("wisdom", convertCharacter->getWisdom());
    setIntValue("wisdomMod", Combatant::getAbilityMod(convertCharacter->getWisdom()));
    setIntValue("wisdomSave", convertCharacter->getSkillBonus(Combatant::Skills_wisdomSave));
    setIntValue("charisma", convertCharacter->getCharisma());
    setIntValue("charismaMod", Combatant::getAbilityMod(convertCharacter->getCharisma()));
    setIntValue("charismaSave", convertCharacter->getSkillBonus(Combatant::Skills_charismaSave));

    setStringValue("race", convertCharacter->getStringValue(Character::StringValue_race));
    setStringValue("subrace", convertCharacter->getStringValue(Character::StringValue_subrace));
    setStringValue("movement", QString::number(convertCharacter->getIntValue(Character::IntValue_speed)));
    setStringValue("class", convertCharacter->getStringValue(Character::StringValue_class));
    setStringValue("level", QString::number(convertCharacter->getIntValue(Character::IntValue_level)));
    setStringValue("experience", convertCharacter->getStringValue(Character::StringValue_experience));
    setStringValue("maximumHp", QString::number(convertCharacter->getIntValue(Character::IntValue_level)));
    // setStringValue("tempHp", QString(""));
    setStringValue("eyes", convertCharacter->getStringValue(Character::StringValue_eyes));
    setStringValue("age", convertCharacter->getStringValue(Character::StringValue_age));
    setStringValue("sex", convertCharacter->getStringValue(Character::StringValue_sex));
    setStringValue("hair", convertCharacter->getStringValue(Character::StringValue_hair));
    setStringValue("height", convertCharacter->getStringValue(Character::StringValue_height));
    setStringValue("weight", convertCharacter->getStringValue(Character::StringValue_weight));
    setStringValue("alignment", convertCharacter->getStringValue(Character::StringValue_alignment));
    setStringValue("background", convertCharacter->getStringValue(Character::StringValue_background));
    // setBoolValue("inspiration", false);

    setIntValue("acrobatics", convertCharacter->getSkillBonus(Combatant::Skills_acrobatics));
    setIntValue("animalHandling", convertCharacter->getSkillBonus(Combatant::Skills_animalHandling));
    setIntValue("arcana", convertCharacter->getSkillBonus(Combatant::Skills_arcana));
    setIntValue("athletics", convertCharacter->getSkillBonus(Combatant::Skills_athletics));
    setIntValue("deception", convertCharacter->getSkillBonus(Combatant::Skills_deception));
    setIntValue("history", convertCharacter->getSkillBonus(Combatant::Skills_history));
    setIntValue("insight", convertCharacter->getSkillBonus(Combatant::Skills_insight));
    setIntValue("intimidation", convertCharacter->getSkillBonus(Combatant::Skills_intimidation));
    setIntValue("investigation", convertCharacter->getSkillBonus(Combatant::Skills_investigation));
    setIntValue("medicine", convertCharacter->getSkillBonus(Combatant::Skills_medicine));
    setIntValue("nature", convertCharacter->getSkillBonus(Combatant::Skills_nature));
    setIntValue("perception", convertCharacter->getSkillBonus(Combatant::Skills_perception));
    setIntValue("performance", convertCharacter->getSkillBonus(Combatant::Skills_performance));
    setIntValue("persuasion", convertCharacter->getSkillBonus(Combatant::Skills_persuasion));
    setIntValue("religion", convertCharacter->getSkillBonus(Combatant::Skills_religion));
    setIntValue("sleightOfHand", convertCharacter->getSkillBonus(Combatant::Skills_sleightOfHand));
    setIntValue("stealth", convertCharacter->getSkillBonus(Combatant::Skills_stealth));
    setIntValue("survival", convertCharacter->getSkillBonus(Combatant::Skills_survival));

    setIntValue("passiveperception", 10 + convertCharacter->getSkillBonus(Combatant::Skills_perception));

    setStringValue("pactmagiclevel", QString::number(convertCharacter->getIntValue(Character::IntValue_pactMagicLevel)));
    setResourceValue("pactmagicslots", ResourcePair(convertCharacter->getIntValue(Character::IntValue_pactMagicUsed), convertCharacter->getIntValue(Character::IntValue_pactMagicSlots)));

    setIntValue("platinum", convertCharacter->getIntValue(Character::IntValue_platinum));
    setIntValue("gold", convertCharacter->getIntValue(Character::IntValue_gold));
    // setIntValue("electrum", 0);
    setIntValue("silver", convertCharacter->getIntValue(Character::IntValue_silver));
    setIntValue("copper", convertCharacter->getIntValue(Character::IntValue_copper));

    setStringValue("spell-data", convertCharacter->getSpellString());
    setStringValue("equipment", convertCharacter->getStringValue(Character::StringValue_equipment));
    setStringValue("proficiencies", convertCharacter->getStringValue(Character::StringValue_proficiencies));
    setStringValue("notes", convertCharacter->getStringValue(Character::StringValue_notes));

    // Set up the spell slots
    if(convertCharacter->spellSlotLevels() > 0)
    {
        QList<QVariant> listValues;

        for(int i = 1; i <= convertCharacter->spellSlotLevels(); ++i)
        {
            int spellSlotCount = convertCharacter->getSpellSlots(i);
            int spellSlotsUsed = convertCharacter->getSpellSlotsUsed(i);
            if(spellSlotCount > 0)
            {
                QHash<QString, QVariant> listEntryValues;
                listEntryValues.insert(QString("level"), QVariant(i));
                QVariant resourceVariant;
                resourceVariant.setValue(ResourcePair(spellSlotsUsed, spellSlotCount));
                listEntryValues.insert(QString("slots"), resourceVariant);
                listValues.append(listEntryValues);
            }
        }

        setValue(QString("spellSlots"), listValues);
    }

    QList<MonsterAction> actions = convertCharacter->getActions();

    if(!actions.isEmpty())
    {
        QList<QVariant> actionValues;
        for(int i = 0; i < actions.size(); ++i)
        {
            QHash<QString, QVariant> actionEntry;
            actionEntry["name"] = actions[i].getName();
            actionEntry["desc"] = actions[i].getDescription();
            actionEntry["attack_bonus"] = actions[i].getAttackBonus();
            QVariant damageVariant;
            damageVariant.setValue(actions[i].getDamageDice());
            actionEntry["damage"] = damageVariant;

            actionValues.append(actionEntry);
        }

        setValue(QString("actions"), actionValues);
    }
}
