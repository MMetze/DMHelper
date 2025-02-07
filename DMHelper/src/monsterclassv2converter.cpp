#include "monsterclassv2converter.h"
#include "monsterclass.h"
#include "monsteraction.h"
#include <QDomElement>

MonsterClassv2Converter::MonsterClassv2Converter(const QDomElement &element) :
    MonsterClassv2{QString(), nullptr}
{
    MonsterClass* oldMonster = new MonsterClass(element, false);

    beginBatchChanges();
    convertValues(oldMonster);
    endBatchChanges();

    delete oldMonster;
}

void MonsterClassv2Converter::convertValues(MonsterClass* monsterClass)
{
    if(!monsterClass)
        return;

    // Walk through the MonsterClass object and convert the values to the MonsterClassv2 object
    setPrivate(monsterClass->getPrivate());
    setStringValue("name", monsterClass->getName());
    setStringValue("type", monsterClass->getMonsterType());
    setStringValue("subtype", monsterClass->getMonsterSubType());
    setStringValue("size", monsterClass->getMonsterSize());
    setStringValue("speed", monsterClass->getSpeed());
    setStringValue("alignment", monsterClass->getAlignment());
    setStringValue("languages", monsterClass->getLanguages());
    setIntValue("armor_class", monsterClass->getArmorClass());
    setDiceValue("hit_dice", monsterClass->getHitDice());
    setIntValue("hit_points", monsterClass->getAverageHitPoints());
    setStringValue("condition_immunities", monsterClass->getConditionImmunities());
    setStringValue("damage_immunities", monsterClass->getDamageImmunities());
    setStringValue("damage_resistances", monsterClass->getDamageResistances());
    setStringValue("damage_vulnerabilities", monsterClass->getDamageVulnerabilities());
    setStringValue("senses", monsterClass->getSenses());
    setStringValue("skills", monsterClass->getSkillString());
    setIntValue("challenge_rating", monsterClass->getChallenge().toInt());
    setIntValue("experience", monsterClass->getXP());
    setIntValue("legendary", monsterClass->getLegendary() ? 3 : 0);

    setIntValue("strength", monsterClass->getStrength());
    setIntValue("strengthMod", Combatant::getAbilityMod(monsterClass->getStrength()));
    setIntValue("strengthSave", monsterClass->getSkillValue(Combatant::Skills_strengthSave));
    setIntValue("dexterity", monsterClass->getDexterity());
    setIntValue("dexterityMod", Combatant::getAbilityMod(monsterClass->getDexterity()));
    setIntValue("dexteritySave", monsterClass->getSkillValue(Combatant::Skills_dexteritySave));
    setIntValue("constitution", monsterClass->getConstitution());
    setIntValue("constitutionMod", Combatant::getAbilityMod(monsterClass->getConstitution()));
    setIntValue("constitutionSave", monsterClass->getSkillValue(Combatant::Skills_constitutionSave));
    setIntValue("intelligence", monsterClass->getIntelligence());
    setIntValue("intelligenceMod", Combatant::getAbilityMod(monsterClass->getIntelligence()));
    setIntValue("intelligenceSave", monsterClass->getSkillValue(Combatant::Skills_intelligenceSave));
    setIntValue("wisdom", monsterClass->getWisdom());
    setIntValue("wisdomMod", Combatant::getAbilityMod(monsterClass->getWisdom()));
    setIntValue("wisdomSave", monsterClass->getSkillValue(Combatant::Skills_wisdomSave));
    setIntValue("charisma", monsterClass->getCharisma());
    setIntValue("charismaMod", Combatant::getAbilityMod(monsterClass->getCharisma()));
    setIntValue("charismaSave", monsterClass->getSkillValue(Combatant::Skills_charismaSave));

    // Read the actions from the MonsterClass object
    convertList(QString("actions"), monsterClass->getActions());
    convertList(QString("legendary_actions"), monsterClass->getLegendaryActions());
    convertList(QString("special_abilities"), monsterClass->getSpecialAbilities());
    convertList(QString("reactions"), monsterClass->getReactions());

    // Iterate through the monsterClass icons and add them tho this object
    for(const auto& icon : monsterClass->getIconList())
    {
        addIcon(icon);
    }
}

void MonsterClassv2Converter::convertList(const QString& listName, const QList<MonsterAction>& actionList)
{
    if(!_factory)
        return;

    // Iterate through the list and create the individual attributes
    for(const auto& action : actionList)
    {
        QHash<QString, QVariant> actionValues;

        actionValues.insert("name", action.getName());
        actionValues.insert("desc", action.getDescription());
        actionValues.insert("attack_bonus", action.getAttackBonus());
        QVariant diceVariant;
        diceVariant.setValue(action.getDamageDice());
        actionValues.insert("damage", diceVariant);

        // Add the action to the list
        appendListEntry(listName, actionValues);
    }
}
