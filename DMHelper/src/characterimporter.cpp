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

bool CharacterImporter::interpretReply(QNetworkReply* reply)
{
    if((!_campaign) || (reply->error() != QNetworkReply::NoError))
        return false;

    QByteArray bytes = reply->readAll();

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
    _character->setHitPoints(rootObject["baseHitPoints"].toInt(1));
    _character->setIntValue(Character::IntValue_experience, rootObject["currentXp"].toInt(0));

    QJsonObject raceObject = rootObject["race"].toObject();
    _character->setStringValue(Character::StringValue_race, raceObject["fullName"].toString());

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
    QJsonArray inventoryArray = rootObject["inventory"].toArray();
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

    // TODO: alignment, classes & levels

    _character->endBatchChanges();

    emit characterImported(_character->getID());


    QString avatarUrl = rootObject["avatarUrl"].toString();
    disconnect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::replyFinished);
    connect(_manager, &QNetworkAccessManager::finished, this, &CharacterImporter::imageReplyFinished);

    _manager->get(QNetworkRequest(QUrl(avatarUrl)));

    return true;
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
            QString avatarFile = QFileDialog::getSaveFileName(nullptr, QString("Select a filename and location to save the character's avatar image"), QString(), QString("Images (*.png)"));
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
    delete this;
}
