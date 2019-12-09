#include "characterimporter.h"
#include "campaign.h"
#include "character.h"
#include "characterimportdialog.h"
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
#include <QDebug>

//#define LOCAL_IMPORTER_TEST

CharacterImporter::CharacterImporter(QObject *parent) :
    QObject(parent),
    _manager(nullptr),
    _campaign(nullptr),
    _character(nullptr)
{
}

CharacterImporter::~CharacterImporter()
{
    delete _manager;
}

void CharacterImporter::importCharacter(Campaign* campaign)
{
    CharacterImportDialog dlg;
    int result = dlg.exec();
    QString characterId = dlg.getCharacterId();
    if((result == QDialog::Rejected) || (characterId.isEmpty()))
    {
        delete this;
        return;
    }

    _manager = new QNetworkAccessManager(this);
    connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::replyFinished);

    _campaign = campaign;
    QString urlString = QString("https://www.dndbeyond.com/character/") + characterId + QString("/json");
    _manager->get(QNetworkRequest(QUrl(urlString)));
}

QUuid CharacterImporter::oldImportCharacter(Campaign& campaign)
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
    _character = campaign.getCharacterByDndBeyondId(dndBeyondId);
    if(!_character)
    {
        _character = new Character();
        _character->setDndBeyondID(dndBeyondId);
        campaign.addCharacter(_character);
    }

    _character->beginBatchChanges();

    _character->setName(characterObject["name"].toString());
    _character->setStringValue(Character::StringValue_sex, characterObject["gender"].toString());
    _character->setStringValue(Character::StringValue_age, QString::number(characterObject["age"].toInt()));
    _character->setStringValue(Character::StringValue_hair, characterObject["hair"].toString());
    _character->setStringValue(Character::StringValue_eyes, characterObject["eyes"].toString());
    _character->setStringValue(Character::StringValue_height, characterObject["height"].toString());
    _character->setStringValue(Character::StringValue_weight, QString::number(characterObject["weight"].toInt()));
    _character->setHitPoints(characterObject["baseHitPoints"].toInt(1));
    _character->setIntValue(Character::IntValue_experience, characterObject["currentXp"].toInt(0));

    QJsonObject raceObject = characterObject["race"].toObject();
    _character->setStringValue(Character::StringValue_race, raceObject["fullName"].toString());

    // TODO stats - "entityTypeId":1472902489, bonus stats & override stats?#
    QJsonArray statsArray = characterObject["stats"].toArray();
    for(i = 0; i < statsArray.count(); ++i)
    {
        QJsonObject statObject = statsArray.at(i).toObject();
        Character::IntValue statId = static_cast<Character::IntValue>(statObject["id"].toInt(0) + 2); // adjustment to fit the IntValue enum
        int statValue = statObject["value"].toInt(0);
        if((statId >= Character::IntValue_strength) && (statId <= Character::IntValue_charisma))
        {
            _character->setIntValue(statId, statValue);
        }
    }

    QJsonObject modifiersObject = characterObject["modifiers"].toObject();
    scanModifiers(modifiersObject, QString("race"), *_character);
    scanModifiers(modifiersObject, QString("background"), *_character);
    scanModifiers(modifiersObject, QString("class"), *_character);
    scanModifiers(modifiersObject, QString("item"), *_character);
    scanModifiers(modifiersObject, QString("feat"), *_character);
    scanModifiers(modifiersObject, QString("condition"), *_character);

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
    totalArmor += Combatant::getAbilityMod(_character->getDexterity());
    _character->setArmorClass(totalArmor);

    // TODO: alignment, classes & levels, equipment, spells



    _character->endBatchChanges();

    return _character->getID();
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
#ifndef LOCAL_IMPORTER_TEST
    if((!_campaign) || (reply->error() != QNetworkReply::NoError))
        return false;

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
    qDebug() << "[Character Importer] Importing Character: " << rootObject["name"].toString();

    int dndBeyondId = rootObject["id"].toInt(DMH_GLOBAL_INVALID_ID);
    _character = _campaign->getCharacterByDndBeyondId(dndBeyondId);
    if(!_character)
    {
        _character = new Character();
        _character->setDndBeyondID(dndBeyondId);
        _campaign->addCharacter(_character);
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

    QJsonObject raceObject = rootObject["race"].toObject();
    _character->setStringValue(Character::StringValue_race, raceObject["fullName"].toString());
    QJsonObject weightSpeedObj = raceObject["weightSpeeds"].toObject();
    QJsonObject normalSpeedObj = weightSpeedObj["normal"].toObject();
    _character->setIntValue(Character::IntValue_speed, normalSpeedObj["walk"].toInt(30));

    // TODO stats - "entityTypeId":1472902489, bonus stats & override stats?
    int i;
    QJsonArray statsArray = rootObject["stats"].toArray();
    for(i = 0; i < statsArray.count(); ++i)
    {
        QJsonObject statObject = statsArray.at(i).toObject();
        Character::IntValue statId = static_cast<Character::IntValue>(statObject["id"].toInt(0) + 2); // adjustment to fit the IntValue enum
        int statValue = statObject["value"].toInt(0);
        if((statId >= Character::IntValue_strength) && (statId <= Character::IntValue_charisma))
        {
            _character->setIntValue(statId, statValue);
        }
    }

    QJsonObject modifiersObject = rootObject["modifiers"].toObject();
    scanModifiers(modifiersObject, QString("race"), *_character);
    scanModifiers(modifiersObject, QString("background"), *_character);
    scanModifiers(modifiersObject, QString("class"), *_character);
    scanModifiers(modifiersObject, QString("item"), *_character);
    scanModifiers(modifiersObject, QString("feat"), *_character);
    scanModifiers(modifiersObject, QString("condition"), *_character);

    // TODO AC - 10 or inventory/definition/armorClass for equipped:true, grantedModifiers, "entityTypeId":112130694 for
    bool hasArmor = false;
    int totalArmor = 0;
    QString equipmentStr;
    QJsonArray inventoryArray = rootObject["inventory"].toArray();
    for(i = 0; i < inventoryArray.count(); ++i)
    {
        QJsonObject inventoryObj = inventoryArray.at(i).toObject();
        QJsonObject invDescription = inventoryObj["definition"].toObject();
        if(inventoryObj["equipped"].toBool())
        {
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

        QString itemDesc = invDescription["name"].toString();
        int quantity = inventoryObj["quantity"].toInt(0);
        if(quantity > 1)
            itemDesc += QString(" (") + QString::number(quantity) + QString(")");
        equipmentStr += itemDesc + QChar::LineFeed;
    }
    if(!hasArmor)
        totalArmor += 10;
    totalArmor += Combatant::getAbilityMod(_character->getDexterity());
    _character->setArmorClass(totalArmor);
    _character->setStringValue(Character::StringValue_equipment, equipmentStr);

    // Initiative
    _character->setInitiative(Combatant::getAbilityMod(_character->getDexterity()));

    // Read the classes and levels
    QString classString;
    QString classFeatureString;
    QVector<int> spellSlots(9,0);
    int levelCount = 0;
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
        levelCount += classLevel;

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
    _character->setIntValue(Character::IntValue_level, levelCount);

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
            spellString += QString("Level ") + QString::number(i) + QString(": ");
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
    featuresString += QChar::LineFeed;
    featuresString += QString("Class Traits") + QChar::LineFeed;
    featuresString += classFeatureString;
    _character->setStringValue(Character::StringValue_proficiencies, featuresString);

    // Hit points
    int totalHP = rootObject["baseHitPoints"].toInt(0);
    totalHP += rootObject["bonusHitPoints"].toInt(0);
    totalHP += rootObject["overrideHitPoints"].toInt(0);
    totalHP += rootObject["temporaryHitPoints"].toInt(0);
    totalHP -= rootObject["removedHitPoints"].toInt(0);
    totalHP += levelCount * Combatant::getAbilityMod(_character->getAbilityValue(Combatant::Ability_Constitution));
    if(totalHP == 0)
        totalHP = 1;
    _character->setHitPoints(totalHP);

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

    // TODO: alignment, classes & levels


    QString avatarUrl = rootObject["avatarUrl"].toString();
    if(!avatarUrl.isEmpty())
    {
        disconnect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::replyFinished);
        connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::imageReplyFinished);
        _manager->get(QNetworkRequest(QUrl(avatarUrl)));
    }
    else
    {
        _character->endBatchChanges();
        emit characterImported(_character->getID());
    }

#ifndef LOCAL_IMPORTER_TEST
    return true;
#else
    TEMPFILE.close();
    return false;
#endif
}

bool CharacterImporter::interpretImageReply(QNetworkReply* reply)
{
    if((!_campaign) || (!_character) || (reply->error() != QNetworkReply::NoError))
        return false;

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

    return true;
}

void CharacterImporter::replyFinished(QNetworkReply *reply)
{
    if(!interpretReply(reply))
        delete this;
}

void CharacterImporter::imageReplyFinished(QNetworkReply *reply)
{
    interpretImageReply(reply);

    _character->endBatchChanges();
    emit characterImported(_character->getID());

    delete this;
}
