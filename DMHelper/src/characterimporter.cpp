#include "characterimporter.h"
#include "campaign.h"
#include "characterimportdialog.h"
#include "combatantfactory.h"
#include "spellbook.h"
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QSslSocket>
#include <QFileDialog>
#include <QApplication>
#include <QAbstractButton>
#include <QDebug>

//#define IMPORTER_LOCAL_TEST
//#define IMPORTER_LOG_AC
//#define IMPORTER_LOG_HP

const int MulticlassSpellslots[20][9] = {
    {2, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 0, 0, 0, 0, 0, 0, 0, 0},
    {4, 2, 0, 0, 0, 0, 0, 0, 0},
    {4, 3, 0, 0, 0, 0, 0, 0, 0},
    {4, 3, 2, 0, 0, 0, 0, 0, 0},
    {4, 3, 3, 0, 0, 0, 0, 0, 0},
    {4, 3, 3, 1, 0, 0, 0, 0, 0},
    {4, 3, 3, 2, 0, 0, 0, 0, 0},
    {4, 3, 3, 3, 1, 0, 0, 0, 0},
    {4, 3, 3, 3, 2, 0, 0, 0, 0},
    {4, 3, 3, 3, 2, 1, 0, 0, 0},
    {4, 3, 3, 3, 2, 1, 0, 0, 0},
    {4, 3, 3, 3, 2, 1, 1, 0, 0},
    {4, 3, 3, 3, 2, 1, 1, 0, 0},
    {4, 3, 3, 3, 2, 1, 1, 1, 0},
    {4, 3, 3, 3, 2, 1, 1, 1, 0},
    {4, 3, 3, 3, 2, 1, 1, 1, 1},
    {4, 3, 3, 3, 3, 1, 1, 1, 1},
    {4, 3, 3, 3, 3, 2, 1, 1, 1},
    {4, 3, 3, 3, 3, 2, 2, 1, 1}
};

CharacterImporter::CharacterImporter(QObject *parent) :
    QObject(parent),
    _manager(nullptr),
    _reply(nullptr),
    _campaign(nullptr),
    _character(nullptr),
    _characterList(),
    _isCharacter(true),
    _msgBox(nullptr),
    _attributeSetValues(),
    _levelCount(0),
    _totalArmor(0),
    _unarmored(0),
    _speedModifier(0),
    _totalHP(0),
    _halfProficiency(false),
    _overrideList(),
    _overrideHP(false)
{
    initializeValues();
}

CharacterImporter::~CharacterImporter()
{
    delete _msgBox;
    delete _manager;
}

void CharacterImporter::importCharacter(Campaign* campaign, bool isCharacter)
{
    if(!campaign)
    {
        qDebug() << "[CharacterImporter] ERROR: no valid campaign found for import!";
        return;
    }

    if((_character) || (!_characterList.isEmpty()))
    {
        qDebug() << "[CharacterImporter] ERROR: character import aborted because update is ongoing!";
        return;
    }

    CharacterImportDialog dlg;
    int result = dlg.exec();
    QString characterId = dlg.getCharacterId();
    if((result == QDialog::Rejected) || (characterId.isEmpty()))
    {
        qDebug() << "[CharacterImporter] Character import cancelled by user.";
        deleteLater();
        return;
    }

    _isCharacter = isCharacter;
    _campaign = campaign;

    startImport(characterId);
}

void CharacterImporter::updateCharacter(Characterv2* character)
{
    if((!character) || (character->getDndBeyondID() == -1))
    {
        qDebug() << "[CharacterImporter] ERROR: no valid character provided for update: " << character;
        return;
    }

    //_character = character;
    _characterList.append(character);
    //QString characterId = QString::number(_character->getDndBeyondID());
    _campaign = dynamic_cast<Campaign*>(character->getParentByType(DMHelper::CampaignType_Campaign));

    startImport();
}

void CharacterImporter::updateCharacters(QList<Characterv2*> characters)
{
    if(characters.isEmpty())
    {
        qDebug() << "[CharacterImporter] ERROR: no characters provided for update!";
        return;
    }

    for(Characterv2* character : characters)
    {
        if((character) && (character->getDndBeyondID() != -1))
            _characterList.append(character);
    }

    if(_characterList.isEmpty())
    {
        qDebug() << "[CharacterImporter] ERROR: no valid characters or DnD Beyond characters provided for update!";
        return;
    }

    //_character = character;
    //QString characterId = QString::number(_character->getDndBeyondID());
    if(_characterList.at(0))
        _campaign = dynamic_cast<Campaign*>(_characterList.at(0)->getParentByType(DMHelper::CampaignType_Campaign));

    startImport();
}

void CharacterImporter::campaignChanged()
{
    _campaign = nullptr;
}

void CharacterImporter::scanStats(QJsonArray statsArray, QJsonArray bonusStatsArray, QJsonArray overrideStatsArray, Characterv2& character)
{
    scanSingleStat(1, statsArray, bonusStatsArray, overrideStatsArray, character);
    scanSingleStat(2, statsArray, bonusStatsArray, overrideStatsArray, character);
    scanSingleStat(3, statsArray, bonusStatsArray, overrideStatsArray, character);
    scanSingleStat(4, statsArray, bonusStatsArray, overrideStatsArray, character);
    scanSingleStat(5, statsArray, bonusStatsArray, overrideStatsArray, character);
    scanSingleStat(6, statsArray, bonusStatsArray, overrideStatsArray, character);
}

void CharacterImporter::scanSingleStat(int statId, QJsonArray statsArray, QJsonArray bonusStatsArray, QJsonArray overrideStatsArray, Characterv2& character)
{
    int finalStatValue = 0;
    int overrideValue = getStatValue(overrideStatsArray, statId);
    if(overrideValue > 0)
    {
        _overrideList.append(statId);
        finalStatValue = overrideValue;
    }
    else
    {
        finalStatValue = getStatValue(statsArray, statId) + getStatValue(bonusStatsArray, statId);
    }

    QString statName = getStatName(statId);
    character.setIntValue(statName, finalStatValue);
    character.setIntValue(statName + QString("Mod"), Combatant::getAbilityMod(finalStatValue));
}

int CharacterImporter::getStatValue(QJsonArray statValueArray, int statIdValue)
{
    for(int i = 0; i < statValueArray.count(); ++i)
    {
        QJsonObject statObject = statValueArray.at(i).toObject();
        int statId = statObject["id"].toInt(0);
        if(statId == statIdValue)
            return statObject["value"].toInt(0);
    }

    return 0;
}

void CharacterImporter::setStatMods(Characterv2& character)
{
    character.setIntValue(QString("strengthMod"), Combatant::getAbilityMod(character.getIntValue(QString("strength"))));
    character.setIntValue(QString("strengthSave"), character.getIntValue(QString("strengthSave")) + character.getIntValue(QString("strengthMod")));

    character.setIntValue(QString("dexterityMod"), Combatant::getAbilityMod(character.getIntValue(QString("dexterity"))));
    character.setIntValue(QString("dexteritySave"), character.getIntValue(QString("dexteritySave")) + character.getIntValue(QString("dexterityMod")));

    character.setIntValue(QString("constitutionMod"), Combatant::getAbilityMod(character.getIntValue(QString("constitution"))));
    character.setIntValue(QString("constitutionSave"), character.getIntValue(QString("constitutionSave")) + character.getIntValue(QString("constitutionMod")));

    character.setIntValue(QString("intelligenceMod"), Combatant::getAbilityMod(character.getIntValue(QString("intelligence"))));
    character.setIntValue(QString("intelligenceSave"), character.getIntValue(QString("intelligenceSave")) + character.getIntValue(QString("intelligenceMod")));

    character.setIntValue(QString("wisdomMod"), Combatant::getAbilityMod(character.getIntValue(QString("wisdom"))));
    character.setIntValue(QString("wisdomSave"), character.getIntValue(QString("wisdomSave")) + character.getIntValue(QString("wisdomMod")));

    character.setIntValue(QString("charismaMod"), Combatant::getAbilityMod(character.getIntValue(QString("charisma"))));
    character.setIntValue(QString("charismaSave"), character.getIntValue(QString("charismaSave")) + character.getIntValue(QString("charismaMod")));
}

QString CharacterImporter::getStatName(int statId)
{
    switch(statId)
    {
        case 1: return QString("strength");
        case 2: return QString("dexterity");
        case 3: return QString("constitution");
        case 4: return QString("intelligence");
        case 5: return QString("wisdom");
        case 6: return QString("charisma");
        default: return QString();
    }
}

void CharacterImporter::scanModifiers(QJsonObject modifiersObject, const QString& key, Characterv2& character)
{
    int proficiencyBonus = DMHelper::ProficiencyProgression[character.getIntValue(QString("level")) - 1];

    QJsonArray modifiersArray = modifiersObject.value(key).toArray();
    for(int i = 0; i < modifiersArray.count(); ++i)
    {
        QJsonObject modObject = modifiersArray.at(i).toObject();
        QString modType = modObject["type"].toString();
        QString modSubType = modObject["subType"].toString();
        QString modFriendlySubType = modObject["friendlySubtypeName"].toString();
        int modValue = modObject["value"].toInt(0);
        if(modObject["entityTypeId"].toInt() == 1472902489) // attribute
        {
            int statId = modObject["entityId"].toInt();
            if((statId >= 1) && (statId <= 6) && (!_overrideList.contains(statId)))
            {
                QString statName = getStatName(statId);
                if(modType == QString("set"))
                    _attributeSetValues[statId] = modValue;
                else
                    character.setIntValue(statName, character.getIntValue(statName) + modValue);
            }
        }
        else if(modType == QString("proficiency"))
        {
            if(CombatantFactory::Instance()->hasAttribute(modSubType))
            {
                character.setIntValue(modSubType, character.getIntValue(modSubType) + proficiencyBonus);
            }
            else if(modSubType == QString("sleight-of-hand"))
            {
                character.setIntValue(QString("sleightOfHand"), character.getIntValue(QString("sleightOfHand")) + proficiencyBonus);
            }
            else if(modSubType == QString("animal-handling"))
            {
                character.setIntValue(QString("animalHandling"), character.getIntValue(QString("animalHandling")) + proficiencyBonus);
            }
            else
            {
                if(modFriendlySubType == QString("Strength Saving Throws"))
                    character.setIntValue("strengthSave", proficiencyBonus);
                else if(modFriendlySubType == QString("Dexterity Saving Throws"))
                    character.setIntValue("dexteritySave", proficiencyBonus);
                else if(modFriendlySubType == QString("Constitution Saving Throws"))
                    character.setIntValue("constitutionSave", proficiencyBonus);
                else if(modFriendlySubType == QString("Intelligence Saving Throws"))
                    character.setIntValue("intelligenceSave", proficiencyBonus);
                else if(modFriendlySubType == QString("Wisdom Saving Throws"))
                    character.setIntValue("wisdomSave", proficiencyBonus);
                else if(modFriendlySubType == QString("Charisma Saving Throws"))
                    character.setIntValue("charismaSave", proficiencyBonus);
            }
        }
        else if(modType == QString("expertise"))
        {
            if(CombatantFactory::Instance()->hasAttribute(modSubType))
            {
                character.setIntValue(modSubType, character.getIntValue(modSubType) + proficiencyBonus);
            }
            else if(modSubType == QString("sleight-of-hand"))
            {
                character.setIntValue(QString("sleightOfHand"), character.getIntValue(QString("sleightOfHand")) + proficiencyBonus);
            }
            else if(modSubType == QString("animal-handling"))
            {
                character.setIntValue(QString("animalHandling"), character.getIntValue(QString("animalHandling")) + proficiencyBonus);
            }
        }
        else if(modType == QString("set"))
        {
            if(modFriendlySubType == QString("Unarmored Armor Class"))
            {
                int statValue = modObject["statId"].toInt(0);
                if(statValue > 0)
                    _unarmored = Combatant::getAbilityMod(_character->getAbilityValue(static_cast<Combatant::Ability>(statValue - 1)));
                else
                    _unarmored = modValue;

#ifdef IMPORTER_LOG_AC
                qDebug() << "[CharacterImporter] AC LOG: Unarmored Armor Class: " << _unarmored;
#endif
            }
        }
        else
        {
            if((modType == QString("bonus")) && (modFriendlySubType == QString("Armor Class")))
            {
                _totalArmor += modValue;
#ifdef IMPORTER_LOG_AC
                qDebug() << "[CharacterImporter] AC LOG: Armor class bonus: " << modValue << ", total AC: " << _totalArmor;
#endif
            }
            else if((modType == QString("bonus")) && (modFriendlySubType == QString("Armored Armor Class")))
            {
                _totalArmor++;
#ifdef IMPORTER_LOG_AC
                qDebug() << "[CharacterImporter] AC LOG: Armored Armor class bonus: +1, total AC: " << _totalArmor;
#endif
            }
            else if((modType == QString("bonus")) && ((modFriendlySubType == QString("speed")) || ((modFriendlySubType == QString("unarmored-movement")))))
            {
                _speedModifier += modValue;
            }
            else if((modType == QString("half-proficiency")) && (modFriendlySubType == QString("Initiative")))
            {
                _halfProficiency = true;
            }
            else if((modType == QString("bonus")) && (modFriendlySubType == QString("Hit Points per Level")) && (!_overrideHP))
            {
                _totalHP += modValue * _levelCount;
#ifdef IMPORTER_LOG_HP
                qDebug() << "[CharacterImporter] HP LOG: Bonus mod HP per Level: " << modValue << " * " << _levelCount << " levels, total HP: " << _totalHP;
#endif
            }
        }
    }
}

void CharacterImporter::addAction(QList<QVariant>& actionValues, const QString& actionName, const QString& actionDescription, int attackBonus, Dice damage)
{
    QHash<QString, QVariant> actionEntry;
    actionEntry["name"] = actionName;
    actionEntry["desc"] = actionDescription;
    actionEntry["attack_bonus"] = attackBonus;
    QVariant damageVariant;
    damageVariant.setValue(damage);
    actionEntry["damage"] = damageVariant;

    actionValues.append(actionEntry);
}

void CharacterImporter::scanChoices(QJsonObject choicesObject, Characterv2& character)
{
    // TODO: Are choices necessary, don't we find proficiencies elsewhere?
    /*
    QStringList choicesKeys = choicesObject.keys();
    for(QString key : choicesKeys)
    {
        QJsonArray choicesArray = choicesObject.value(key).toArray();
        for(int i = 0; i < choicesArray.count(); ++i)
        {
            QJsonObject choiceObject = choicesArray.at(i).toObject();
            int choiceType = choiceObject["type"].toInt(0);
            int optionValue = choiceObject["optionValue"].toInt(0);
            if((choiceType == 2) && (optionValue > 0)) // Proficiency type and value set
            {
                QJsonArray optionsArray = choiceObject.value("options").toArray();
                for(int j = 0; j < optionsArray.count(); ++j)
                {
                    QJsonObject optionObject = optionsArray.at(j).toObject();
                    int optionId = optionObject["id"].toInt(0);
                    if(optionId == optionValue)
                    {
                        QString optionLabel = optionObject["label"].toString().toLower();
                        if(CombatantFactory::Instance()->hasAttribute(optionLabel))
                        {
                            character.setIntValue(modSubType, character.getIntValue(modSubType) + proficiencyBonus);
                        }
                        else if(modSubType == QString("sleight of hand"))
                        {
                            character.setIntValue(QString("sleightOfHand"), character.getIntValue(QString("sleightOfHand")) + proficiencyBonus);
                        }
                        else if(modSubType == QString("animal handling"))
                        {
                            character.setIntValue(QString("animalHandling"), character.getIntValue(QString("animalHandling")) + proficiencyBonus);
                        }
                        // TODO: add ELSE for other proficiencies that can be added to the character proficiency list
                    }
                }
            }
        }
    }
                        */
}

QString CharacterImporter::getNotesString(QJsonObject notesParent, const QString& key, const QString& title)
{
    QString result;

    QString notesStr = notesParent[key].toString();
    if(!notesStr.isEmpty())
    {
        result += title + QString("<br>");
        result += notesStr.replace(QString("\n"), QString("<br>"));
        result += QString("<br>") + QString("<br>");
    }

    return result;
}

QString CharacterImporter::getSpellString(QJsonObject rootObject)
{

    QVector<QStringList> spellVector(10);

    int i;
    QJsonArray classSpellsArray = rootObject["classSpells"].toArray();
    for(i = 0; i < classSpellsArray.count(); ++i)
    {
        QJsonObject classSpellObject = classSpellsArray.at(i).toObject();
        //int entityTypeId = classSpellObject["entityTypeId"].toInt();
        int characterClassId = classSpellObject["characterClassId"].toInt();

        bool classFound = false;
        bool preparesSpells = false;
        QJsonArray classesArray = rootObject["classes"].toArray();
        int k = 0;
        while((!classFound) && (i < classesArray.count()))
        {
            QJsonObject classObject = classesArray.at(k).toObject();
            int classId = classObject["id"].toInt();

            if(classId == characterClassId)
            {
                QJsonObject classDefnObj = classObject["definition"].toObject();
                preparesSpells = (classDefnObj["spellPrepareType"].toInt() == 1);

                classFound = true;
            }
            ++k;
        }

        QJsonArray classSpellArray = classSpellObject["spells"].toArray();
        parseSpellSource(spellVector, rootObject, classSpellArray, !preparesSpells);
    }

    QJsonObject generalSpells = rootObject["spells"].toObject();
    QJsonArray generalRaceArray = generalSpells["race"].toArray();
    parseSpellSource(spellVector, rootObject, generalRaceArray, true);
    QJsonArray generalClassArray = generalSpells["class"].toArray();
    parseSpellSource(spellVector, rootObject, generalClassArray, true);
    QJsonArray generalItemArray = generalSpells["item"].toArray();
    parseSpellSource(spellVector, rootObject, generalItemArray, true);
    QJsonArray generalFeatArray = generalSpells["feat"].toArray();
    parseSpellSource(spellVector, rootObject, generalFeatArray, true);

    QString spellString;
    for(i = 0; i < spellVector.size(); ++i)
    {
        if(spellVector.at(i).count() > 0)
        {
            spellString += ((i == 0) ? QString("<b>Cantrips</b>") : (QString("<b>Level ") + QString::number(i) + QString("</b>"))) + QString("<br>");
            spellVector[i].sort();
            spellString += spellVector.at(i).join(QString("<br>"));
            spellString += QString("<br>");
            spellString += QString("<br>");
        }
    }

    return spellString;
}

void CharacterImporter::parseSpellSource(QVector<QStringList>& spellVector, QJsonObject rootObject, QJsonArray spellSource, bool autoPrepared)
{
    for(int i = 0; i < spellSource.count(); ++i)
    {
        QJsonObject spellObject = spellSource.at(i).toObject();
        bool prepared = spellObject["prepared"].toBool();
        bool alwaysPrepared= spellObject["alwaysPrepared"].toBool();
        QJsonObject definitionObject = spellObject["definition"].toObject();
        int spellLevel = definitionObject["level"].toInt();
        QString spellName = definitionObject["name"].toString();

        if(((spellLevel == 0) || (autoPrepared) || (prepared) || (alwaysPrepared)) && ((!spellName.isEmpty()) && (spellLevel >= 0)))
        {
            // Make sure there's enough space in the vector
            if(spellLevel > spellVector.size() + 1)
                spellVector.resize(spellLevel + 1);

            bool spellExists = ((Spellbook::Instance()) && (Spellbook::Instance()->exists(spellName)));

            QString vectorName = QString("   ");
            if(spellExists)
                vectorName += QString("<a href=") + spellName + QString(">");
            vectorName += spellName;
            int componentId = spellObject["componentId"].toInt();
            int componentTypeId = spellObject["componentTypeId"].toInt();
            if(componentId != 0)
            {
                QString itemName = getEquipmentName(rootObject, componentId, componentTypeId);
                if(!itemName.isEmpty())
                    vectorName.append(QString(" <i>(") + itemName + QString(")</i>"));
            }
            if(spellExists)
                vectorName += QString("</a>");
            spellVector[spellLevel].append(vectorName);
        }
    }
}

QString CharacterImporter::getEquipmentName(QJsonObject rootObject, int itemId, int itemTypeId)
{
    if(itemId == 0)
        return QString();

    QJsonArray inventoryArray = rootObject["inventory"].toArray();
    for(int i = 0; i < inventoryArray.count(); ++i)
    {
        QJsonObject inventoryObject = inventoryArray.at(i).toObject();
        QJsonObject definitionObject = inventoryObject["definition"].toObject();
        int inventoryObjectId = definitionObject["id"].toInt();
        int inventoryObjectTypeId = definitionObject["entityTypeId"].toInt();
        QString inventoryObjectName = definitionObject["name"].toString();
        if((inventoryObjectId == itemId) && (inventoryObjectTypeId == itemTypeId))
            return inventoryObjectName;
    }

    return QString();
}

bool CharacterImporter::interpretReply(QNetworkReply* reply)
{
    if(!reply)
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An unexpected error occured connecting to Dnd Beyond."));
        qDebug() << "[CharacterImporter] ERROR: unexpected null pointer in network reply";
        return false;
    }

#ifndef IMPORTER_LOCAL_TEST
    if(reply->error() != QNetworkReply::NoError)
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An error occured connecting to Dnd Beyond:") + QChar::LineFeed + reply->errorString());
        qDebug() << "[CharacterImporter] ERROR: network reply not ok: " << reply->error();
        qDebug() << "[CharacterImporter] ERROR: " << reply->errorString();
        return false;
    }

    QByteArray bytes = reply->readAll();
#else
    Q_UNUSED(reply);
    QFile TEMPFILE("C://Users//Craig//Documents//GitHub//DMHelper//DMHelper//testdata//cyrus.json");
    if(!TEMPFILE.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QByteArray bytes = TEMPFILE.readAll();
#endif

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if(doc.isNull())
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("Unable to parse the input string\n") + err.errorString());
        qDebug() << "[CharacterImporter] ERROR: " << err.error << " in column " << err.offset;
        qDebug() << "[CharacterImporter] ERROR: " << err.errorString();
        return false;
    }

    QJsonObject rootObject = doc.object();
    qDebug() << "[CharacterImporter] Reply from DnD Beyond received, parsing character: " << rootObject["name"].toString();

    bool isUpdate = _character != nullptr;
    if(!isUpdate)
    {
        if(!_campaign)
            return false;

        int dndBeyondId = rootObject["id"].toInt(DMH_GLOBAL_INVALID_ID);
        // HACK
        _character = nullptr;
        //_character = _campaign->getCharacterOrNPCByDndBeyondId(dndBeyondId);
        if((!_character) && (CombatantFactory::Instance()))
        {
            _character = dynamic_cast<Characterv2*>(CombatantFactory::Instance()->createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, QString(), false));
            _character->setDndBeyondID(dndBeyondId);
        }
    }

    _character->beginBatchChanges();

    /* // TODO: make sure all values are imported
        <dmh:attribute type="dmh:integer" name="conditions"/>
        <dmh:attribute type="dmh:integer" name="passiveperception"/>
        <dmh:element type="dmh:list" name="hitDice"><dmh:attribute type="dmh:string" name="class"/><dmh:attribute type="dmh:dice" name="diceType"/><dmh:attribute type="dmh:resource" name="diceCount"/></dmh:element>
        <dmh:element type="dmh:list" name="actions"><dmh:element type="dmh:string" name="name"/><dmh:element type="dmh:string" name="desc"/><dmh:element type="dmh:integer" name="attack_bonus"/><dmh:element type="dmh:dice" name="damage"/></dmh:element>
    */

    _character->setName(rootObject["name"].toString());
    _character->setStringValue(QString("sex"), rootObject["gender"].toString());
    _character->setStringValue(QString("age"), QString::number(rootObject["age"].toInt()));
    _character->setStringValue(QString("hair"), rootObject["hair"].toString());
    _character->setStringValue(QString("eyes"), rootObject["eyes"].toString());
    _character->setStringValue(QString("height"), rootObject["height"].toString());
    _character->setStringValue(QString("weight"), QString::number(rootObject["weight"].toInt()));
    _character->setStringValue(QString("experience"), rootObject["currentXp"].toString());
    _character->setBoolValue(QString("inspiration"), rootObject["inspiration"].toBool(false));

    int alignmentId = rootObject["alignmentId"].toInt(1);
    switch(alignmentId)
    {
        case 1: _character->setStringValue(QString("alignment"), QString("Lawful Good")); break;
        case 2: _character->setStringValue(QString("alignment"), QString("Neutral Good")); break;
        case 3: _character->setStringValue(QString("alignment"), QString("Chaotic Good")); break;
        case 4: _character->setStringValue(QString("alignment"), QString("Lawful Neutral")); break;
        case 5: _character->setStringValue(QString("alignment"), QString("True Neutral")); break;
        case 6: _character->setStringValue(QString("alignment"), QString("Chaotic Neutral")); break;
        case 7: _character->setStringValue(QString("alignment"), QString("Lawful Evil")); break;
        case 8: _character->setStringValue(QString("alignment"), QString("Neutral Evil")); break;
        case 9: _character->setStringValue(QString("alignment"), QString("Chaotic Evil")); break;
        default:
            break;
    };

    // Read the race
    QJsonObject raceObject = rootObject["race"].toObject();
    _character->setStringValue(QString("race"), raceObject["baseName"].toString()); // Alternatives: "subRaceShortName": and "baseName":
    _character->setStringValue(QString("subrace"), raceObject["subRaceShortName"].toString()); // Alternatives: "subRaceShortName": and "baseName":
    QJsonObject weightSpeedObj = raceObject["weightSpeeds"].toObject();
    QJsonObject normalSpeedObj = weightSpeedObj["normal"].toObject();
    int normal_speed_walk = normalSpeedObj["walk"].toInt(30);
    int normal_speed_fly = normalSpeedObj["fly"].toInt(30);
    int normal_speed_burrow = normalSpeedObj["burrow"].toInt(30);
    int normal_speed_swim = normalSpeedObj["swim"].toInt(30);
    int normal_speed_climb = normalSpeedObj["climb"].toInt(30);

    QString sizeString = raceObject["size"].toString();
    _character->setStringValue(QString("size"), sizeString);
    QString raceFeatureString;
    int i;
    QJsonArray raceFeatureArray = raceObject["racialTraits"].toArray();
    for(i = 0; i < raceFeatureArray.count(); ++i)
    {
        QJsonObject featureObject = raceFeatureArray.at(i).toObject();
        QJsonObject featureDefnObject = featureObject["definition"].toObject();
        raceFeatureString += QString("<b>") + featureDefnObject["name"].toString() + QString(":</b><br>");
        QString description = featureDefnObject["description"].toString();
        if(!description.isEmpty())
            raceFeatureString += description + QString("<br>");
        QString snippet = featureDefnObject["snippet"].toString();
        if(!snippet.isEmpty())
            raceFeatureString += snippet + QString("<br>");
    }

    // Read the background
    QJsonObject backgroundObject = rootObject["background"].toObject();
    QJsonObject backgroundDefinitionObject = backgroundObject["definition"].toObject();
    _character->setStringValue(QString("background"), backgroundDefinitionObject["name"].toString());
    QString backgroundFeatureString;
    QString backgroundFeatureName = backgroundDefinitionObject["featureName"].toString();
    QString backgroundFeatureDescription = backgroundDefinitionObject["featureDescription"].toString();
    if(!backgroundFeatureName.isEmpty())
    {
        backgroundFeatureString += QString("<b>") + backgroundFeatureName + QString("</b>");
        if(!backgroundFeatureDescription.isEmpty())
            backgroundFeatureString += QString(":<br>") + backgroundFeatureDescription;
        backgroundFeatureString += QString("<br>");
    }
    QString backgroundTools = backgroundDefinitionObject["toolProficienciesDescription"].toString();
    if(!backgroundTools.isEmpty())
    {
        backgroundFeatureString += QString("<b>Tool Proficiencies:</b><br>") + backgroundTools + QString("<br>");
    }

    // TODO stats - "entityTypeId":1472902489, bonus stats & override stats?
    QJsonArray statsArray = rootObject["stats"].toArray();
    QJsonArray bonusStatsArray = rootObject["bonusStats"].toArray();
    QJsonArray overrideStatsArray = rootObject["overrideStats"].toArray();
    scanStats(statsArray, bonusStatsArray, overrideStatsArray, *_character);

    // Read the classes and levels
    QString classString;
    QString classFeatureString;
    QVector<int> spellSlots(9, 0);
    //QVector<int> pactMagicSlots(5, 0);
    int pactMagicSlots;
    int pactMagicUsed;
    int pactMagicLevel;
    int multiclassCasterLevel = 0;
    int casterClassCount = 0;
    bool warlockCaster = false;
    int fixedHP = 0;
    QJsonArray classesArray = rootObject["classes"].toArray();
    for(i = 0; i < classesArray.count(); ++i)
    {
        QJsonObject classObject = classesArray.at(i).toObject();
        if(i > 0)
            classString += QString("/");

        QJsonObject classDefnObj = classObject["definition"].toObject();
        QString className = classDefnObj["name"].toString();
        classString += className;

        QJsonObject subclassDefnObj = classObject["subclassDefinition"].toObject();
        QString subclassName = subclassDefnObj["name"].toString();
        if(!subclassName.isEmpty())
            classString += QString(" (") + subclassName + QString(")");

        int classLevel = classObject["level"].toInt();
        _levelCount += classLevel;

        bool startingClass = classObject["isStartingClass"].toBool();
        int classHitDice = classDefnObj["hitDice"].toInt(0);
        int classHP = 0;
        if(startingClass)
            classHP = classHitDice + ((classLevel - 1) * ((classHitDice / 2) + 1));
        else
            classHP = classLevel * ((classHitDice / 2) + 1);

        fixedHP += classHP;
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Class " << classString << ", starting: " << startingClass << ", hitdice: " << classHitDice << ", level: " << classLevel << ", total class HP: " << classHP << ", total fixed HP: " << fixedHP;
#endif

        QJsonArray featureArray = classObject["classFeatures"].toArray();
        for(int j = 0; j < featureArray.count(); ++j)
        {
            QJsonObject featureObject = featureArray.at(j).toObject();
            QJsonObject featureDefnObject = featureObject["definition"].toObject();
            int levelReq = featureDefnObject["requiredLevel"].toInt();
            if(classLevel >= levelReq)
            {
                classFeatureString += QString("<b>") + featureDefnObject["name"].toString() + QString("</b>");
                QString snippet = featureDefnObject["snippet"].toString();
                if(!snippet.isEmpty())
                    classFeatureString += QString(":<br>") + snippet;
                classFeatureString += QString("<br>");
            }
        }

        QJsonObject spellRulesObject = classDefnObj["spellRules"].toObject();
        bool canCastSpells = classDefnObj["canCastSpells"].toBool(false);
        bool subclassCanCastSpells = subclassDefnObj["canCastSpells"].toBool(false);
        int multiClassSpellSlotDivisor = spellRulesObject["multiClassSpellSlotDivisor"].toInt();
        if(className == QString("Warlock"))
        {
            warlockCaster = true;
            QJsonArray spellArray = spellRulesObject["levelSpellSlots"].toArray();
            if(spellArray.count() > classLevel)
            {
                QJsonArray spellSlotsAvailable = spellArray.at(classLevel).toArray();
                for(int k = 0; k < 5; ++k)
                {
                    if(spellSlotsAvailable.at(k).toInt() > 0)
                    {
                        pactMagicSlots = spellSlotsAvailable.at(k).toInt();
                        pactMagicLevel = k + 1;
                    }

                    //pactMagicSlots[k] = spellSlotsAvailable.at(k).toInt();
                }
            }
        }
        else
        {
            if((canCastSpells) || (subclassCanCastSpells))
            {
                if(multiclassCasterLevel == 0)
                {
                    QJsonArray spellArray = spellRulesObject["levelSpellSlots"].toArray();
                    if(spellArray.count() > classLevel)
                    {
                        QJsonArray spellSlotsAvailable = spellArray.at(classLevel).toArray();
                        for(int k = 0; k < 9; ++k)
                            spellSlots[k] = spellSlotsAvailable.at(k).toInt();
                    }
                }

                multiclassCasterLevel += classLevel / multiClassSpellSlotDivisor;
                ++casterClassCount;
            }
        }
    }
    _character->setStringValue(QString("class"), classString);
    _character->setStringValue(QString("level"), QString::number(_levelCount));
    int proficiencyBonus = DMHelper::ProficiencyProgression[_levelCount - 1];

    // Fill in multi-classes spell slots
    if((casterClassCount > 1) && ((multiclassCasterLevel > 0) && (multiclassCasterLevel <= 20)))
    {
        for(int k = 0; k < 9; ++k)
            spellSlots[k] = MulticlassSpellslots[multiclassCasterLevel - 1][k];
    }

    // Read various modifiers
    QJsonObject modifiersObject = rootObject["modifiers"].toObject();
    scanModifiers(modifiersObject, QString("race"), *_character);
    scanModifiers(modifiersObject, QString("background"), *_character);
    scanModifiers(modifiersObject, QString("class"), *_character);
    // THESE ARE NOT RELIABLY APPLIED: scanModifiers(modifiersObject, QString("item"), *_character);
    scanModifiers(modifiersObject, QString("feat"), *_character);
    scanModifiers(modifiersObject, QString("condition"), *_character);

    // Jack of all trades - TODO maybe re-add this?
    //if(_halfProficiency)
    //    _character->setIntValue((static_cast<Character::IntValue>(Character::IntValue_jackofalltrades)), 1);

    // Set the movement values
    _character->setIntValue(QString("speed"), normal_speed_walk + _speedModifier);
    QString movementString = QString("Walk ") + QString::number(normal_speed_walk + _speedModifier);
    if(normal_speed_swim > 0)
        movementString += QString(", Swim ") + QString::number(normal_speed_swim + _speedModifier);
    if(normal_speed_fly > 0)
        movementString += QString(", Fly ") + QString::number(normal_speed_fly + _speedModifier);
    if(normal_speed_climb > 0)
        movementString += QString(", Climb ") + QString::number(normal_speed_climb + _speedModifier);
    if(normal_speed_burrow > 0)
        movementString += QString(", Burrow ") + QString::number(normal_speed_burrow + _speedModifier);
    _character->setStringValue(QString("movement"), movementString);

    // Parse the choices section
    scanChoices(rootObject["choices"].toObject(), *_character);

    // The list for collecting "actions"
    QList<QVariant> actionValues;

    // TODO AC - 10 or inventory/definition/armorClass for equipped:true, grantedModifiers, "entityTypeId":112130694 for
    // Scan the inventory
    QString armorType;
    QString equipmentStr;
    QJsonArray inventoryArray = rootObject["inventory"].toArray();
    for(i = 0; i < inventoryArray.count(); ++i)
    {
        QJsonObject inventoryObj = inventoryArray.at(i).toObject();
        QJsonObject invDefinition = inventoryObj["definition"].toObject();
        if((inventoryObj["equipped"].toBool()) &&
           ((!inventoryObj["canAttune"].toBool()) ||
            (inventoryObj["attunementDescription"].toString() != QString("Required")) ||
            (inventoryObj["isAttuned"].toBool())))
        {
            if(invDefinition["filterType"].toString() == QString("Weapon"))
            {
                QString weaponName = invDefinition["name"].toString();
                QString weaponDescription = invDefinition["description"].toString();

                int j;
                bool finesse = false;
                bool ranged = false;
                QJsonArray propertyArray = invDefinition["properties"].toArray();
                for(j = 0; j < propertyArray.count(); ++j)
                {
                    QJsonObject propertyObj = propertyArray.at(j).toObject();
                    if(propertyObj["name"].toString() == QString("Finesse"))
                        finesse = true;
                    else if(propertyObj["name"].toString() == QString("Ranged"))
                        ranged = true;
                }

                // Get the attack bonus
                int attackBonus = proficiencyBonus;
                if(ranged)
                    attackBonus += Combatant::getAbilityMod(_character->getDexterity());
                else
                {
                    if(finesse)
                        attackBonus += qMax(Combatant::getAbilityMod(_character->getStrength()), Combatant::getAbilityMod(_character->getDexterity()));
                    else
                        attackBonus += Combatant::getAbilityMod(_character->getStrength());
                }

                // Get the basic damage
                Dice weaponDamage;
                QJsonObject damageObject = invDefinition["damage"].toObject();
                if(!damageObject.isEmpty())
                    weaponDamage = Dice(damageObject["diceString"].toString());

                // Check for magic properties
                int magicBonus = 0;
                QJsonArray grantedModifiers = invDefinition["grantedModifiers"].toArray();
                for(j = 0; j < grantedModifiers.count(); ++j)
                {
                    QJsonObject grantedModifier = grantedModifiers.at(j).toObject();
                    if((grantedModifier["type"].toString() == QString("bonus")) && (grantedModifier["subType"].toString() == QString("magic")))
                        magicBonus += grantedModifier["value"].toInt();
                }
                weaponDamage.setBonus(weaponDamage.getBonus() + magicBonus);

                addAction(actionValues, weaponName, weaponDescription, attackBonus + magicBonus, weaponDamage);
            }
            else
            {
                int armorClass = invDefinition["armorClass"].toInt();
                if(armorClass > 0)
                {
                    if(armorClass >= 10)
                        armorType = invDefinition["type"].toString();
                    _totalArmor += armorClass;
    #ifdef IMPORTER_LOG_AC
                    qDebug() << "[CharacterImporter] AC LOG: " << invDefinition["name"].toString() << ", equipped: " << inventoryObj["equipped"].toBool()
                             << ", canAttune: " << inventoryObj["canAttune"].toBool() << ", attunementDescription: " << inventoryObj["attunementDescription"].toString()
                             << ", isAttuned: " << inventoryObj["isAttuned"].toBool() << ", armorClass: " << invDefinition["armorClass"].toInt()
                             << ", total AC: " << _totalArmor;
    #endif
                }
                scanModifiers(invDefinition, QString("grantedModifiers"), *_character);
            }
        }

        QString itemDesc = invDefinition["name"].toString();
        int quantity = inventoryObj["quantity"].toInt(0);
        if(quantity > 1)
            itemDesc += QString(" (") + QString::number(quantity) + QString(")");
        equipmentStr += itemDesc + QString("<br>");
    }

    // Set the currency/coin values
    QJsonObject currenciesObject = rootObject["currencies"].toObject();
    _character->setIntValue(QString("platinum"), currenciesObject["pp"].toInt());
    _character->setIntValue(QString("gold"), currenciesObject["gp"].toInt());
    _character->setIntValue(QString("electrum"), currenciesObject["ep"].toInt());
    _character->setIntValue(QString("silver"), currenciesObject["sp"].toInt());

    // Find the player's name in the campaign area
    QJsonObject campaignObject = rootObject["campaign"].toObject();
    QJsonArray campaignCharacterArray = campaignObject["characters"].toArray();
    for(i = 0; i < campaignCharacterArray.count(); ++i)
    {
        QJsonObject characterObj = campaignCharacterArray.at(i).toObject();
        if(characterObj["characterName"].toString() == _character->getName())
        {
            _character->setStringValue(QString("player"), characterObj["username"].toString());
            break;
        }
    }

    // Calculate the final stats
    // Check if there are any stats set instead of modified
    QMapIterator<int, int> attrIt(_attributeSetValues);
    while(attrIt.hasNext())
    {
        attrIt.next();
        _character->setIntValue(getStatName(attrIt.key()), attrIt.value());
    }

    setStatMods(*_character);

    // Calculate the final armor class
    if(armorType == QString("Heavy Armor"))
    {
#ifdef IMPORTER_LOG_AC
        qDebug() << "[CharacterImporter] AC LOG: Heavy armor, no dexterity bonus, total AC: " << _totalArmor;
#endif
    }
    else if(armorType == QString("Medium Armor"))
    {
        _totalArmor += qMin(2, Combatant::getAbilityMod(_character->getDexterity()));
#ifdef IMPORTER_LOG_AC
        qDebug() << "[CharacterImporter] AC LOG: Medium armor, Dexterity armor class adjustment (max 2): " << qMin(2, Combatant::getAbilityMod(_character->getDexterity())) << ", total AC: " << _totalArmor;
#endif
    }
    else if(armorType == QString("Light Armor"))
    {
        _totalArmor += Combatant::getAbilityMod(_character->getDexterity());
#ifdef IMPORTER_LOG_AC
        qDebug() << "[CharacterImporter] AC LOG: Light armor, Dexterity armor class adjustment: " << Combatant::getAbilityMod(_character->getDexterity()) << ", total AC: " << _totalArmor;
#endif
    }
    else
    {
        _totalArmor += 10 + Combatant::getAbilityMod(_character->getDexterity());
#ifdef IMPORTER_LOG_AC
        qDebug() << "[CharacterImporter] AC LOG: No armor, based AC only +10 plus Dexterity: " << Combatant::getAbilityMod(_character->getDexterity()) << ", total AC: " << _totalArmor;
#endif
        if(_unarmored != 0)
        {
            _totalArmor += _unarmored;
#ifdef IMPORTER_LOG_AC
            qDebug() << "[CharacterImporter] AC LOG: Unarmored bonus: " << _unarmored << ", total AC: " << _totalArmor;
#endif
        }
    }

    _character->setArmorClass(_totalArmor);
    _character->setStringValue(QString("equipment"), equipmentStr);

    // Calculate the final initiative modifier
    int initiativeValue = Combatant::getAbilityMod(_character->getDexterity());
    if(_halfProficiency)
        initiativeValue += proficiencyBonus / 2;
    _character->setInitiative(initiativeValue);

    // Spell Overview
    if(casterClassCount >= 1)
    {
        QJsonArray spellSlotArray = rootObject["spellSlots"].toArray();
        QList<QVariant> listValues;

        for(int i = 0; i < 9; i++)
        {
            if(spellSlots[i] > 0)
            {
                int usedSlots = 0;
                if(spellSlotArray.count() > i)
                {
                    QJsonObject spellSlotObject = spellSlotArray.at(i).toObject();
                    usedSlots += spellSlotObject["used"].toInt();
                }

                QHash<QString, QVariant> listEntryValues;
                listEntryValues.insert(QString("level"), QVariant(i + 1));
                QVariant resourceVariant;
                resourceVariant.setValue(ResourcePair(usedSlots, spellSlots[i]));
                listEntryValues.insert(QString("slots"), resourceVariant);
                listValues.append(listEntryValues);
            }
        }

        _character->setValue(QString("spellSlots"), listValues);
    }

    if(warlockCaster)
    {
        QJsonArray pactSlotArray = rootObject["pactMagic"].toArray();

        if(pactSlotArray.count() >= pactMagicLevel)
        {
            QJsonObject spellSlotObject = pactSlotArray.at(pactMagicLevel - 1).toObject();
            pactMagicUsed = (spellSlotObject["used"].toInt() > pactMagicSlots) ? pactMagicSlots : spellSlotObject["used"].toInt();

            _character->setStringValue(QString("pactmagiclevel"), QString::number(pactMagicLevel));
            _character->setResourceValue(QString("pactmagicslots"), ResourcePair(pactMagicUsed, pactMagicSlots));
        }
    }

    //_character->setSpellString(getSpellString(rootObject));
    _character->setStringValue(QString("spell-data"), getSpellString(rootObject));

    // Find actions
    QJsonObject actionsObject = rootObject["actions"].toObject();
    QJsonArray raceActionsArray = actionsObject["race"].toArray();
    for(i = 0; i < raceActionsArray.count(); ++i)
    {
        QJsonObject actionObject = raceActionsArray.at(i).toObject();
        addAction(actionValues, actionObject["name"].toString(), actionObject["snippet"].toString(), 0, Dice());
    }
    QJsonArray classActionsArray = actionsObject["class"].toArray();
    for(i = 0; i < classActionsArray.count(); ++i)
    {
        QJsonObject actionObject = classActionsArray.at(i).toObject();
        addAction(actionValues, actionObject["name"].toString(), actionObject["snippet"].toString(), 0, Dice());

        // Special case for bardic inspiration
        if(actionObject["name"].toString() == QString("Bardic Inspiration"))
        {
            QJsonObject usageObject = actionObject["limitedUse"].toObject();
            if(!usageObject.isEmpty())
            {
                int numberUsed = usageObject["numberUsed"].toInt();
                int statModifierUsesId = usageObject["statModifierUsesId"].toInt();
                int numberUsages = _character->getIntValue(getStatName(statModifierUsesId) + QString("Mod"));
                _character->setResourceValue(QString("bardic-inspiration"), ResourcePair(numberUsed, numberUsages));
            }
        }
    }
    QJsonArray featActionsArray = actionsObject["feat"].toArray();
    for(i = 0; i < featActionsArray.count(); ++i)
    {
        QJsonObject actionObject = featActionsArray.at(i).toObject();
        addAction(actionValues, actionObject["name"].toString(), actionObject["snippet"].toString(), 0, Dice());
    }

    // Features Overview
    QString featuresString = QString("<b>Feats</b>") + QString("<br>");
    QJsonArray featsArray = rootObject["feats"].toArray();
    for(i = 0; i < featsArray.count(); ++i)
    {
        QJsonObject featObject = featsArray.at(i).toObject();
        QJsonObject featDefnObject = featObject["definition"].toObject();
        featuresString += featDefnObject["name"].toString() + QString("<br>");
    }
    if(!classFeatureString.isEmpty())
        featuresString += QString("<br>") + QString("<b>Class Traits</b>") + QString("<br>") + classFeatureString;
    if(!raceFeatureString.isEmpty())
        featuresString += QString("<br>")+ QString("<b>Racial Traits</b>") + QString("<br>") + raceFeatureString;
    if(!backgroundFeatureString.isEmpty())
        featuresString += QString("<br>") + QString("<b>Background Features</b>") + QString("<br>") + backgroundFeatureString;
    _character->setStringValue(QString("proficiencies"), featuresString);

    // Calculate the final Hit points
    int currentHP;
    int overrideHPValue = rootObject["overrideHitPoints"].toInt(0);
    if(overrideHPValue > 0)
    {
        _totalHP = overrideHPValue;
        currentHP = _totalHP;
        _overrideHP = true;
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Override hit points: " << overrideHPValue;
#endif
    }
    else
    {
        QJsonObject preferencesObject = rootObject["preferences"].toObject();
        int hpMode = preferencesObject["hitPointType"].toInt(0);
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Hit point mode: " << hpMode;
#endif

        int baseHitPoints = rootObject["baseHitPoints"].toInt(0);
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Base hit points: " << rootObject["baseHitPoints"].toInt(0);
#endif
        if(hpMode == 1)
        {
            _totalHP += fixedHP;
#ifdef IMPORTER_LOG_HP
            qDebug() << "[CharacterImporter] HP LOG: FIXED hit points used: " << fixedHP << ", total HP: " << _totalHP;
#endif
        }
        else
        {
            _totalHP += baseHitPoints;
#ifdef IMPORTER_LOG_HP
            qDebug() << "[CharacterImporter] HP LOG: Base hit points total HP: " << _totalHP;
#endif
        }

        _totalHP += rootObject["bonusHitPoints"].toInt(0);
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Bonus hit points: " << rootObject["bonusHitPoints"].toInt(0) << ", total HP: " << _totalHP;
#endif

        _totalHP += _levelCount * Combatant::getAbilityMod(_character->getAbilityValue(Combatant::Ability_Constitution));
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Constitution bonus: " << Combatant::getAbilityMod(_character->getAbilityValue(Combatant::Ability_Constitution)) << " * " << _levelCount << " levels, total HP: " << _totalHP;
#endif
        if(_totalHP == 0)
            _totalHP = 1;

        currentHP = _totalHP;
        currentHP += rootObject["temporaryHitPoints"].toInt(0);
        _character->setIntValue(QString("tempHp"), rootObject["temporaryHitPoints"].toInt(0));
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Temporary hit points: " << rootObject["temporaryHitPoints"].toInt(0) << ", current HP: " << currentHP;
#endif

        currentHP -= rootObject["removedHitPoints"].toInt(0);
#ifdef IMPORTER_LOG_HP
        qDebug() << "[CharacterImporter] HP LOG: Removed hit points: " << rootObject["removedHitPoints"].toInt(0) << ", current HP: " << currentHP;
#endif
        if(currentHP == 0)
            currentHP = 1;
    }
    _character->setHitPoints(currentHP);
    _character->setIntValue(QString("maximumHp"), _totalHP);

    // Notes
    QString notesStr;
    QJsonObject notesObject = rootObject["notes"].toObject();
    notesStr += getNotesString(notesObject, QString("allies"), QString("<b>Allies</b>"));
    notesStr += getNotesString(notesObject, QString("personalPossessions"), QString("<b>Personal Possessions</b>"));
    notesStr += getNotesString(notesObject, QString("otherHoldings"), QString("<b>Other Holdings</b>"));
    notesStr += getNotesString(notesObject, QString("organizations"), QString("<b>Organizations</b>"));
    notesStr += getNotesString(notesObject, QString("enemies"), QString("<b>Enemies</b>"));
    notesStr += getNotesString(notesObject, QString("backstory"), QString("<b>Backstory</b>"));
    notesStr += getNotesString(notesObject, QString("otherNotes"), QString("<b>Other Notes</b>"));
    _character->setStringValue(QString("notes"), notesStr);

    if(!isUpdate)
        emit characterCreated(_character);

    QString avatarUrl = rootObject["avatarUrl"].toString();
    if((isUpdate) && (!avatarUrl.isEmpty()))
    {
        QMessageBox::StandardButton result = QMessageBox::question(nullptr,
                                                                   QString("Confirm avatar download"),
                                                                   QString("The updated character has an avatar image. Do you want to update the image?"));
        if(result != QMessageBox::Yes)
            avatarUrl = QString();
    }

    if(!avatarUrl.isEmpty())
    {
        qDebug() << "[CharacterImporter] Parsing character completed, requesting image from: " << avatarUrl;
        disconnect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::replyFinished);
        connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::imageReplyFinished);
        _reply = _manager->get(QNetworkRequest(QUrl(avatarUrl)));

        #ifndef IMPORTER_LOCAL_TEST
            return true;
        #endif
    }
    else
    {
        qDebug() << "[CharacterImporter] Parsing character completed, no image download requested";
        _character->endBatchChanges();
        emit characterImported(_character->getID());

        #ifndef IMPORTER_LOCAL_TEST
            return false;
        #endif
    }

#ifdef IMPORTER_LOCAL_TEST
    TEMPFILE.close();
    return false;
#endif
}

bool CharacterImporter::interpretImageReply(QNetworkReply* reply)
{
    if((!_character) || (!reply))
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An unexpected error occured connecting to Dnd Beyond."));
        qDebug() << "[CharacterImporter] ERROR: unexpected null pointer in network image reply";
        return false;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An error occured connecting to Dnd Beyond:") + QChar::LineFeed + reply->errorString());
        qDebug() << "[CharacterImporter] ERROR: network image reply not ok: " << reply->error();
        qDebug() << "[CharacterImporter] ERROR: " << reply->errorString();
        return false;
    }

    QByteArray bytes = reply->readAll();

    if(bytes.size() > 0)
    {
        QPixmap pmp;
        if(pmp.loadFromData(bytes))
        {
            // TODO: Add the character's name and png as a default filename
            QString avatarFile = QFileDialog::getSaveFileName(nullptr,
                                                              QString("Select a filename and location to save the character's avatar image"),
                                                              QString(_character->getName()) + QString(".png"),
                                                              QString("Images (*.png)"));
            if(!avatarFile.isEmpty())
            {
                pmp.save(avatarFile);
                _character->setIcon(avatarFile);
                // TODO: Make sure this updates directly in the character view!
            }
        }
    }

    qDebug() << "[CharacterImporter] Importing character image completed.";

    return true;
}

void CharacterImporter::startImport(QString characterId)
{
    if(!_manager)
    {
        _manager = new QNetworkAccessManager(this);
        connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::replyFinished);
    }

    if(_msgBox)
        delete _msgBox;

    if((!_character) && (!_characterList.isEmpty()))
        _character = _characterList.takeFirst();

    QString messageStr;
    if(_character)
    {
        characterId = QString::number(_character->getDndBeyondID());
        messageStr = QString("Updating character ") + _character->getName() + QString(" with ID: ") + characterId;
    }
    else
    {
        if(_isCharacter)
            messageStr = QString("Importing character ID: ") + characterId;
        else
            messageStr = QString("Importing NPC ID: ") + characterId;
    }

    qDebug() << "[CharacterImporter] " << messageStr;

    initializeValues();

    _msgBox = new QMessageBox(QMessageBox::Information, "Character Import", messageStr, QMessageBox::Cancel);
    connect(_msgBox, &QMessageBox::buttonClicked, this, &CharacterImporter::messageBoxCancelled);
    _msgBox->show();

    QString urlString = QString("https://www.dndbeyond.com/character/") + characterId + QString("/json");
    _reply = _manager->get(QNetworkRequest(QUrl(urlString)));
}

void CharacterImporter::finishImport()
{
    if(_msgBox)
    {
        _msgBox->deleteLater();
        _msgBox = nullptr;
    }

    if(!_characterList.isEmpty())
    {
        _character = nullptr;
        startImport();
    }
    else
    {
        deleteLater();
    }
}

void CharacterImporter::initializeValues()
{
    _attributeSetValues.clear();
    _levelCount = 0;
    _totalArmor = 0;
    _speedModifier = 0;
    _unarmored = 0;
    _totalHP = 0;
    _halfProficiency = false;
    _overrideList.clear();
    _overrideHP = false;
}

void CharacterImporter::replyFinished(QNetworkReply *reply)
{
    _reply = nullptr;
    if(!interpretReply(reply))
        finishImport();
}

void CharacterImporter::imageReplyFinished(QNetworkReply *reply)
{
    _reply = nullptr;
    interpretImageReply(reply);

    _character->endBatchChanges();
    emit characterImported(_character->getID());

    finishImport();
}

void CharacterImporter::messageBoxCancelled()
{
    if(!_reply)
        return;

    _reply->abort();
}
