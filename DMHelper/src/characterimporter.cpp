#include "characterimporter.h"
#include "campaign.h"
#include "character.h"
#include "characterimportdialog.h"
#include "combatantfactory.h"
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

//#define LOCAL_IMPORTER_TEST

CharacterImporter::CharacterImporter(QObject *parent) :
    QObject(parent),
    _manager(nullptr),
    _reply(nullptr),
    _campaign(nullptr),
    _character(nullptr),
    _isCharacter(true),
    _msgBox(nullptr),
    _attributeSetValues(),
    _levelCount(0),
    _totalArmor(0),
    _totalHP(0),
    _halfProficiency(false)
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
        return;

    CharacterImportDialog dlg;
    int result = dlg.exec();
    QString characterId = dlg.getCharacterId();
    if((result == QDialog::Rejected) || (characterId.isEmpty()))
    {
        deleteLater();
        return;
    }

    _character = nullptr;
    _isCharacter = isCharacter;
    _campaign = campaign;

    startImport(characterId);
}

void CharacterImporter::updateCharacter(Character* character)
{
    if((!character) || (character->getDndBeyondID() == -1))
        return;

    _character = character;
    QString characterId = QString::number(_character->getDndBeyondID());
    _campaign = dynamic_cast<Campaign*>(_character->getParentByType(DMHelper::CampaignType_Campaign));

    startImport(characterId);
}

void CharacterImporter::campaignChanged()
{
    _campaign = nullptr;
}

void CharacterImporter::scanModifiers(QJsonObject modifiersObject, const QString& key, Character& character)
{
    QJsonArray modifiersArray = modifiersObject.value(key).toArray();
    for(int i = 0; i < modifiersArray.count(); ++i)
    {
        QJsonObject modObject = modifiersArray.at(i).toObject();
        QString modType = modObject["type"].toString();
        QString modSubType = modObject["friendlySubtypeName"].toString();
        int modValue = modObject["value"].toInt(0);
        if(modObject["entityTypeId"].toInt() == 1472902489) // attribute
        {
            Character::IntValue statId = static_cast<Character::IntValue>(modObject["entityId"].toInt(0) + 2); // adjustment to fit the IntValue enum
            if((statId >= Character::IntValue_strength) && (statId <= Character::IntValue_charisma))
            {
                if(modType == QString("set"))
                {
                    _attributeSetValues[statId] = modValue;
                }
                else
                {
                    character.setIntValue(statId, character.getIntValue(statId) + modValue);
                }
            }
        }
        else if(modType == QString("proficiency"))
        {
            int skillId = Character::findKeyForSkillName(modSubType);
            if(skillId >= 0)
            {
                character.setSkillValue(static_cast<Combatant::Skills>(skillId), true);
            }
            else
            {
                if(modSubType == QString("Strength Saving Throws"))
                    character.setSkillValue(Combatant::Skills_strengthSave, true);
                else if(modSubType == QString("Dexterity Saving Throws"))
                    character.setSkillValue(Combatant::Skills_dexteritySave, true);
                else if(modSubType == QString("Constitution Saving Throws"))
                    character.setSkillValue(Combatant::Skills_constitutionSave, true);
                else if(modSubType == QString("Intelligence Saving Throws"))
                    character.setSkillValue(Combatant::Skills_intelligenceSave, true);
                else if(modSubType == QString("Wisdom Saving Throws"))
                    character.setSkillValue(Combatant::Skills_wisdomSave, true);
                else if(modSubType == QString("Charisma Saving Throws"))
                    character.setSkillValue(Combatant::Skills_charismaSave, true);
            }
        }
        else if(modType == QString("expertise"))
        {
            int skillId = Character::findKeyForSkillName(modSubType);
            if(skillId >= 0)
            {
                character.setSkillValue(static_cast<Combatant::Skills>(skillId), 2);
            }
        }
        else
        {
            if((modType == QString("bonus")) && (modSubType == QString("Armor Class")))
                _totalArmor += modValue;
            else if((modType == QString("bonus")) && (modSubType == QString("Armored Armor Class")))
                _totalArmor++;
            else if((modType == QString("half-proficiency")) && (modSubType == QString("Initiative")))
                _halfProficiency = true;
            else if((modType == QString("bonus")) && (modSubType == QString("Hit Points per Level")))
                _totalHP += modValue * _levelCount;
        }
    }
}

void CharacterImporter::scanChoices(QJsonObject choicesObject, Character& character)
{
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
                        QString optionLabel = optionObject["label"].toString();
                        //qDebug() << "[Character Importer]           Proficiency found: " << optionLabel;
                        int skillId = Character::findKeyForSkillName(optionLabel);
                        if(skillId >= 0)
                        {
                            character.setSkillValue(static_cast<Combatant::Skills>(skillId), true);
                        }
                        // TODO: add ELSE for other proficiencies that can be added to the character proficiency list
                    }
                }
            }
        }
    }
}

QString CharacterImporter::getNotesString(QJsonObject notesParent, const QString& key, const QString& title)
{
    QString result;

    QString notesStr = notesParent[key].toString();
    if(!notesStr.isEmpty())
    {
        result += title + QChar::LineFeed;
        result += notesStr.replace(QString("\\n"), QString(QChar::LineFeed));
        result += QChar::LineFeed;
        result += QChar::LineFeed;
    }

    return result;
}


bool CharacterImporter::interpretReply(QNetworkReply* reply)
{
    if(!reply)
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An unexpected error occured connecting to Dnd Beyond."));
        qDebug() << "[Character Importer] ERROR: unexpected null pointer in network reply";
        return false;
    }

#ifndef LOCAL_IMPORTER_TEST
    if(reply->error() != QNetworkReply::NoError)
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An error occured connecting to Dnd Beyond:") + QChar::LineFeed + reply->errorString());
        qDebug() << "[Character Importer] ERROR: network reply not ok: " << reply->error();
        qDebug() << "[Character Importer] ERROR: " << reply->errorString();
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
        qDebug() << "[Character Importer] ERROR: " << err.error << " in column " << err.offset;
        qDebug() << "[Character Importer] ERROR: " << err.errorString();
        return false;
    }

    QJsonObject rootObject = doc.object();
    qDebug() << "[Character Importer] Reply from DnD Beyond received, parsing character: " << rootObject["name"].toString();

    bool isUpdate = _character != nullptr;
    if(!isUpdate)
    {
        if(!_campaign)
            return false;

        int dndBeyondId = rootObject["id"].toInt(DMH_GLOBAL_INVALID_ID);
        _character = _campaign->getCharacterOrNPCByDndBeyondId(dndBeyondId);
        if(!_character)
        {
            //_character = new Character();
            _character = dynamic_cast<Character*>(CombatantFactory().createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, QString(), false));
            _character->setDndBeyondID(dndBeyondId);
            /*
            if(_isCharacter)
                _campaign->addCharacter(_character);
            else
                _campaign->addNPC(_character);
                */
            _campaign->addObject(_character);
        }
    }

    _character->beginBatchChanges();

    _character->setName(rootObject["name"].toString());
    _character->setStringValue(Character::StringValue_sex, rootObject["gender"].toString());
    _character->setStringValue(Character::StringValue_age, QString::number(rootObject["age"].toInt()));
    _character->setStringValue(Character::StringValue_hair, rootObject["hair"].toString());
    _character->setStringValue(Character::StringValue_eyes, rootObject["eyes"].toString());
    _character->setStringValue(Character::StringValue_height, rootObject["height"].toString());
    _character->setStringValue(Character::StringValue_weight, QString::number(rootObject["weight"].toInt()));
    _character->setIntValue(Character::IntValue_experience, rootObject["currentXp"].toInt(0));
    _character->setStringValue(Character::StringValue_sex, rootObject["gender"].toString());

    int alignmentId = rootObject["alignmentId"].toInt(1);
    switch(alignmentId)
    {
        case 1: _character->setStringValue(Character::StringValue_alignment, QString("Lawful Good")); break;
        case 2: _character->setStringValue(Character::StringValue_alignment, QString("Neutral Good")); break;
        case 3: _character->setStringValue(Character::StringValue_alignment, QString("Chaotic Good")); break;
        case 4: _character->setStringValue(Character::StringValue_alignment, QString("Lawful Neutral")); break;
        case 5: _character->setStringValue(Character::StringValue_alignment, QString("True Neutral")); break;
        case 6: _character->setStringValue(Character::StringValue_alignment, QString("Chaotic Neutral")); break;
        case 7: _character->setStringValue(Character::StringValue_alignment, QString("Lawful Evil")); break;
        case 8: _character->setStringValue(Character::StringValue_alignment, QString("Neutral Evil")); break;
        case 9: _character->setStringValue(Character::StringValue_alignment, QString("Chaotic Evil")); break;
        default:
            break;
    };

    // Read the race
    QJsonObject raceObject = rootObject["race"].toObject();
    _character->setStringValue(Character::StringValue_race, raceObject["fullName"].toString());
    QJsonObject weightSpeedObj = raceObject["weightSpeeds"].toObject();
    QJsonObject normalSpeedObj = weightSpeedObj["normal"].toObject();
    _character->setIntValue(Character::IntValue_speed, normalSpeedObj["walk"].toInt(30));
    QString sizeString = raceObject["size"].toString();
    _character->setStringValue(Character::StringValue_size, sizeString);
    QString raceFeatureString;
    int i;
    QJsonArray raceFeatureArray = raceObject["racialTraits"].toArray();
    for(i = 0; i < raceFeatureArray.count(); ++i)
    {
        QJsonObject featureObject = raceFeatureArray.at(i).toObject();
        QJsonObject featureDefnObject = featureObject["definition"].toObject();
        raceFeatureString += featureDefnObject["name"].toString() + QString(": ");
        raceFeatureString += featureDefnObject["description"].toString();
        QString snippet = featureDefnObject["snippet"].toString();
        raceFeatureString += QChar::LineFeed;
        if(!snippet.isEmpty())
            raceFeatureString += snippet + QChar::LineFeed;
    }

    // Read the background
    QJsonObject backgroundObject = rootObject["background"].toObject();
    QJsonObject backgroundDefinitionObject = backgroundObject["definition"].toObject();
    _character->setStringValue(Character::StringValue_background, backgroundDefinitionObject["name"].toString());
    QString backgroundFeatureString;
    QString backgroundFeatureName = backgroundDefinitionObject["featureName"].toString();
    QString backgroundFeatureDescription = backgroundDefinitionObject["featureDescription"].toString();
    if(!backgroundFeatureName.isEmpty())
    {
        backgroundFeatureString += backgroundFeatureName;
        if(!backgroundFeatureDescription.isEmpty())
            backgroundFeatureString += QString(": ") + backgroundFeatureDescription;
        backgroundFeatureString += QChar::LineFeed;
    }
    QString backgroundTools = backgroundDefinitionObject["toolProficienciesDescription"].toString();
    if(!backgroundTools.isEmpty())
    {
        backgroundFeatureString += QString("Tools Proficiencies: ") + backgroundTools + QChar::LineFeed;
    }

    // TODO stats - "entityTypeId":1472902489, bonus stats & override stats?
    QJsonArray statsArray = rootObject["stats"].toArray();
    for(i = 0; i < statsArray.count(); ++i)
    {
        QJsonObject statObject = statsArray.at(i).toObject();
        Character::IntValue statId = static_cast<Character::IntValue>(statObject["id"].toInt(0) + 2); // adjustment by 2 to fit the IntValue enum
        int statValue = statObject["value"].toInt(0);
        if((statId >= Character::IntValue_strength) && (statId <= Character::IntValue_charisma))
        {
            _character->setIntValue(statId, statValue);
        }
    }

    // Read the classes and levels
    QString classString;
    QString classFeatureString;
    QVector<int> spellSlots(9,0);
    QJsonArray classesArray = rootObject["classes"].toArray();
    for(i = 0; i < classesArray.count(); ++i)
    {
        QJsonObject classObject = classesArray.at(i).toObject();
        if(i > 0)
            classString += QString("/");

        QJsonObject classDefnObj = classObject["definition"].toObject();
        classString += classDefnObj["name"].toString();

        QJsonObject subclassDefnObj = classObject["subclassDefinition"].toObject();
        QString subclassName = subclassDefnObj["name"].toString();
        if(!subclassName.isEmpty())
            classString += QString(" (") + subclassName + QString(")");

        int classLevel = classObject["level"].toInt();
        _levelCount += classLevel;

        QJsonArray featureArray = classObject["classFeatures"].toArray();
        for(int j = 0; j < featureArray.count(); ++j)
        {
            QJsonObject featureObject = featureArray.at(j).toObject();
            QJsonObject featureDefnObject = featureObject["definition"].toObject();
            int levelReq = featureDefnObject["requiredLevel"].toInt();
            if(classLevel >= levelReq)
            {
                classFeatureString += featureDefnObject["name"].toString();
                QString snippet = featureDefnObject["snippet"].toString();
                if(!snippet.isEmpty())
                    classFeatureString += QString(": ")+ snippet;
                classFeatureString += QChar::LineFeed;
            }
        }

        QJsonObject spellRulesObject = classDefnObj["spellRules"].toObject();
        QJsonArray spellArray = spellRulesObject["levelSpellSlots"].toArray();
        if(spellArray.count() > classLevel)
        {
            QJsonArray spellSlotsAvailable = spellArray.at(classLevel).toArray();
            for(int k = 0; k < 9; ++k)
                spellSlots[k] += spellSlotsAvailable.at(k).toInt();
        }
    }
    _character->setStringValue(Character::StringValue_class, classString);
    _character->setIntValue(Character::IntValue_level, _levelCount);

    // Read various modifiers
    QJsonObject modifiersObject = rootObject["modifiers"].toObject();
    scanModifiers(modifiersObject, QString("race"), *_character);
    scanModifiers(modifiersObject, QString("background"), *_character);
    scanModifiers(modifiersObject, QString("class"), *_character);
    // THESE ARE NOT RELIABLY APPLIED: scanModifiers(modifiersObject, QString("item"), *_character);
    scanModifiers(modifiersObject, QString("feat"), *_character);
    scanModifiers(modifiersObject, QString("condition"), *_character);

    // Jack of all trades
    if(_halfProficiency)
        _character->setIntValue((static_cast<Character::IntValue>(Character::IntValue_jackofalltrades)), 1);

    // Parse the choices section
    scanChoices(rootObject["choices"].toObject(), *_character);

    // TODO AC - 10 or inventory/definition/armorClass for equipped:true, grantedModifiers, "entityTypeId":112130694 for
    // Scan the inventory
    bool hasArmor = false;
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
            int armorClass = invDefinition["armorClass"].toInt();
            if(armorClass > 0)
            {
                if(armorClass >= 10)
                    hasArmor = true;
                _totalArmor += armorClass;
            }
            scanModifiers(invDefinition, QString("grantedModifiers"), *_character);
        }

        QString itemDesc = invDefinition["name"].toString();
        int quantity = inventoryObj["quantity"].toInt(0);
        if(quantity > 1)
            itemDesc += QString(" (") + QString::number(quantity) + QString(")");
        equipmentStr += itemDesc + QChar::LineFeed;
    }

    // Calculate the final stats
    // Check if there are any stats set instead of modified
    QMapIterator<int, int> attrIt(_attributeSetValues);
    while(attrIt.hasNext())
    {
        attrIt.next();
        _character->setIntValue(static_cast<Character::IntValue>(attrIt.key()), attrIt.value());
    }

    // Calculate the final armor class
    if(!hasArmor)
        _totalArmor += 10;
    _totalArmor += Combatant::getAbilityMod(_character->getDexterity());
    _character->setArmorClass(_totalArmor);
    _character->setStringValue(Character::StringValue_equipment, equipmentStr);

    // Calculate the final initiative modifier
    int initiativeValue = Combatant::getAbilityMod(_character->getDexterity());
    if(_halfProficiency)
        initiativeValue += _character->getProficiencyBonus() / 2;
    _character->setInitiative(initiativeValue);

    // Spell Overview
    QString spellString = QString("Spell Slots available") + QChar::LineFeed;
    QJsonArray spellSlotArray = rootObject["spellSlots"].toArray();
    QJsonArray pactSlotArray = rootObject["pactMagic"].toArray();
    for(i = 0; i < 9; ++i)
    {
        if(spellSlots[i] > 0)
        {
            int usedSlots = 0;
            if(spellSlotArray.count() > i)
            {
                QJsonObject spellSlotObject = spellSlotArray.at(i).toObject();
                usedSlots += spellSlotObject["used"].toInt();
            }

            if(spellSlotArray.count() > i)
            {
                QJsonObject spellSlotObject = pactSlotArray.at(i).toObject();
                usedSlots += spellSlotObject["used"].toInt();
            }

            int remainingSlots = (usedSlots > spellSlots[i]) ? 0 : spellSlots[i] - usedSlots;
            spellString += QString("Level ") + QString::number(i + 1) + QString(": ");
            spellString += QString::number(remainingSlots) + QString("/") + QString::number(spellSlots[i]);
            spellString += QChar::LineFeed;
        }
    }
    _character->setStringValue(Character::StringValue_spells, spellString);

    // Features Overview
    QString featuresString = QString("Feats") + QChar::LineFeed;
    QJsonArray featsArray = rootObject["feats"].toArray();
    for(i = 0; i < featsArray.count(); ++i)
    {
        QJsonObject featObject = featsArray.at(i).toObject();
        QJsonObject featDefnObject = featObject["definition"].toObject();
        featuresString += featDefnObject["name"].toString() + QChar::LineFeed;
    }
    if(!classFeatureString.isEmpty())
        featuresString += QChar::LineFeed + QString("Class Traits") + QChar::LineFeed + classFeatureString;
    if(!raceFeatureString.isEmpty())
        featuresString += QChar::LineFeed + QString("Racial Traits") + QChar::LineFeed + raceFeatureString;
    if(!backgroundFeatureString.isEmpty())
        featuresString += QChar::LineFeed + QString("Background Features") + QChar::LineFeed + backgroundFeatureString;
    _character->setStringValue(Character::StringValue_proficiencies, featuresString);

    // Calculate the final Hit points
    _totalHP += rootObject["baseHitPoints"].toInt(0);
    _totalHP += rootObject["bonusHitPoints"].toInt(0);
    _totalHP += rootObject["overrideHitPoints"].toInt(0);
    _totalHP += rootObject["temporaryHitPoints"].toInt(0);
    _totalHP -= rootObject["removedHitPoints"].toInt(0);
    _totalHP += _levelCount * Combatant::getAbilityMod(_character->getAbilityValue(Combatant::Ability_Constitution));
    if(_totalHP == 0)
        _totalHP = 1;
    _character->setHitPoints(_totalHP);

    // Notes
    QString notesStr;
    QJsonObject notesObject = rootObject["notes"].toObject();
    notesStr += getNotesString(notesObject, QString("allies"), QString("Allies"));
    notesStr += getNotesString(notesObject, QString("personalPossessions"), QString("Personal Possessions"));
    notesStr += getNotesString(notesObject, QString("otherHoldings"), QString("Other Holdings"));
    notesStr += getNotesString(notesObject, QString("organizations"), QString("Organizations"));
    notesStr += getNotesString(notesObject, QString("enemies"), QString("Enemies"));
    notesStr += getNotesString(notesObject, QString("backstory"), QString("Backstory"));
    notesStr += getNotesString(notesObject, QString("otherNotes"), QString("Other Notes"));
    _character->setStringValue(Character::StringValue_notes, notesStr);

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
        qDebug() << "[Character Importer] Parsing character completed, requesting image from: " << avatarUrl;
        disconnect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::replyFinished);
        connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::imageReplyFinished);
        _reply = _manager->get(QNetworkRequest(QUrl(avatarUrl)));

        #ifndef LOCAL_IMPORTER_TEST
            return true;
        #endif
    }
    else
    {
        qDebug() << "[Character Importer] Parsing character completed, no image download requested";
        _character->endBatchChanges();
        emit characterImported(_character->getID());

        #ifndef LOCAL_IMPORTER_TEST
            return false;
        #endif
    }

#ifdef LOCAL_IMPORTER_TEST
    TEMPFILE.close();
    return false;
#endif
}

bool CharacterImporter::interpretImageReply(QNetworkReply* reply)
{
    if((!_character) || (!reply))
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An unexpected error occured connecting to Dnd Beyond."));
        qDebug() << "[Character Importer] ERROR: unexpected null pointer in network image reply";
        return false;
    }

    if(reply->error() != QNetworkReply::NoError)
    {
        QMessageBox::critical(nullptr, QString("Character Import Error"), QString("An error occured connecting to Dnd Beyond:") + QChar::LineFeed + reply->errorString());
        qDebug() << "[Character Importer] ERROR: network image reply not ok: " << reply->error();
        qDebug() << "[Character Importer] ERROR: " << reply->errorString();
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

    qDebug() << "[Character Importer] Importing character image completed.";

    return true;
}

void CharacterImporter::startImport(const QString& characterId)
{
    if(!_manager)
    {
        _manager = new QNetworkAccessManager(this);
        connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::replyFinished);
    }

    if(_msgBox)
        delete _msgBox;

    QString messageStr;
    if(_character)
    {
        messageStr = QString("Updating character ") + _character->getName() + QString(" with ID: ") + characterId;
    }
    else
    {
        if(_isCharacter)
            messageStr = QString("Importing character ID: ") + characterId;
        else
            messageStr = QString("Importing NPC ID: ") + characterId;
    }

    qDebug() << "[Character Importer] " << messageStr;

    initializeValues();

    _msgBox = new QMessageBox(QMessageBox::Information,"Character Import", messageStr, QMessageBox::Cancel);
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

    deleteLater();
}

void CharacterImporter::initializeValues()
{
    _attributeSetValues.clear();
    _levelCount = 0;
    _totalArmor = 0;
    _totalHP = 0;
    _halfProficiency = false;
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

