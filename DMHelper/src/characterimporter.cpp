#include "characterimporter.h"
#include "campaign.h"
#include "character.h"
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDebug>

CharacterImporter::CharacterImporter(QObject *parent) :
    QObject(parent)
{
}


QUuid CharacterImporter::importCharacter(Campaign& campaign)
{
    int i;
    bool ok;
    QString characterText = QInputDialog::getMultiLineText(nullptr, QString("Enter Import Character Details"), QString("Character"),QString(), &ok);
    if((!ok) || (characterText.isEmpty()))
        return QUuid();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(characterText.toUtf8(), &err);
    if(doc.isNull())
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("Unable to parse the input string\n") + err.errorString());
        qDebug() << "[Character Importer] ERROR: " << err.error << " in column " << err.offset;
        qDebug() << "[Character Importer] ERROR: " << err.errorString();
        return QUuid();
    }

    QJsonObject rootObject = doc.object();
    if((!rootObject.contains("character")) || (!rootObject["character"].isObject()))
        return QUuid();

    QJsonObject characterObject = rootObject["character"].toObject();

    int dndBeyondId = characterObject["id"].toInt(DMH_GLOBAL_INVALID_ID);
    Character* character = campaign.getCharacterByDndBeyondId(dndBeyondId);
    if(!character)
    {
        character = new Character();
        character->setDndBeyondID(dndBeyondId);
        campaign.addCharacter(character);
    }

    character->beginBatchChanges();

    character->setName(characterObject["name"].toString());
    character->setStringValue(Character::StringValue_sex, characterObject["gender"].toString());
    character->setStringValue(Character::StringValue_age, QString::number(characterObject["age"].toInt()));
    character->setStringValue(Character::StringValue_hair, characterObject["hair"].toString());
    character->setStringValue(Character::StringValue_eyes, characterObject["eyes"].toString());
    character->setStringValue(Character::StringValue_height, characterObject["height"].toString());
    character->setStringValue(Character::StringValue_weight, QString::number(characterObject["weight"].toInt()));
    character->setHitPoints(characterObject["baseHitPoints"].toInt(1));
    character->setIntValue(Character::IntValue_experience, characterObject["currentXp"].toInt(0));

    QJsonObject raceObject = characterObject["race"].toObject();
    character->setStringValue(Character::StringValue_race, raceObject["fullName"].toString());

    // TODO stats - "entityTypeId":1472902489, bonus stats & override stats?#
    QJsonArray statsArray = characterObject["stats"].toArray();
    for(i = 0; i < statsArray.count(); ++i)
    {
        QJsonObject statObject = statsArray.at(i).toObject();
        Character::IntValue statId = static_cast<Character::IntValue>(statObject["id"].toInt(0) + 2); // adjustment to fit the IntValue enum
        int statValue = statObject["value"].toInt(0);
        if((statId >= Character::IntValue_strength) && (statId <= Character::IntValue_charisma))
        {
            character->setIntValue(statId, statValue);
        }
    }

    QJsonObject modifiersObject = characterObject["modifiers"].toObject();
    scanModifiers(modifiersObject, QString("race"), *character);
    scanModifiers(modifiersObject, QString("background"), *character);
    scanModifiers(modifiersObject, QString("class"), *character);
    scanModifiers(modifiersObject, QString("item"), *character);
    scanModifiers(modifiersObject, QString("feat"), *character);
    scanModifiers(modifiersObject, QString("condition"), *character);

    // TODO AC - 10 or inventory/definition/armorClass for equipped:true, grantedModifiers, "entityTypeId":112130694 for
    bool hasArmor = false;
    int totalArmor = 0;
    QJsonArray inventoryArray = characterObject["inventory"].toArray();
    for(i = 0; i < inventoryArray.count(); ++i)
    {
        QJsonObject inventoryObj = inventoryArray.at(i).toObject();
        if(inventoryObj["equipped"].toBool())
        {
            QJsonObject invDescription = inventoryObj["definition"].toObject();
            int armorClass = invDescription["armorClass"].toInt();
            if(armorClass > 0)
            {
                if(armorClass >= 10)
                    hasArmor = true;
                totalArmor += armorClass;
            }
            QJsonArray grantedModArray = invDescription["grantedModifiers"].toArray();
            for(int j = 0; j < grantedModArray.count(); ++j)
            {
                QJsonObject modObject = grantedModArray.at(j).toObject();
                if((modObject["type"].toString() == QString("bonus")) && (modObject["subType"].toString() == QString("armor-class")))
                {
                    totalArmor += modObject["value"].toInt();
                }
            }
        }
    }
    if(!hasArmor)
        totalArmor += 10;
    totalArmor += Combatant::getAbilityMod(character->getDexterity());
    character->setArmorClass(totalArmor);

    // TODO: alignment, classes & levels



    character->endBatchChanges();

    return character->getID();
}

void CharacterImporter::scanModifiers(QJsonObject modifiersObject, const QString& key, Character& character)
{
    QJsonArray modifiersArray = modifiersObject.value(key).toArray();
    for(int i = 0; i < modifiersArray.count(); ++i)
    {
        QJsonObject modObject = modifiersArray.at(i).toObject();
        if(modObject["entityTypeId"].toInt() == 1472902489)
        {
            Character::IntValue statId = static_cast<Character::IntValue>(modObject["entityId"].toInt(0) + 2); // adjustment to fit the IntValue enum
            int statValue = modObject["value"].toInt(0);
            if((statId >= Character::IntValue_strength) && (statId <= Character::IntValue_charisma))
            {
                character.setIntValue(statId, character.getIntValue(statId) + statValue);
            }
        }
    }
}
