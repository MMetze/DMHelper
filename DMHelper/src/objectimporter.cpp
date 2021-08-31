#include "objectimporter.h"
#include "campaign.h"
#include "dmconstants.h"
#include "character.h"
#include "encounterfactory.h"
#include "map.h"
#include "audiotrack.h"
#include <QDomDocument>
#include <QFileDialog>
#include <QStandardItem>
#include <QTextStream>
#include <QMessageBox>
#include <QScopedPointer>
#include <QDebug>

ObjectImporter::ObjectImporter(QObject *parent) :
    QObject(parent),
    _duplicateObjects()
{
}

bool ObjectImporter::importObject(Campaign& campaign)
{
    _duplicateObjects.clear();

    QString filename = QFileDialog::getOpenFileName(nullptr,QString("Select file for importing"), QString(), QString("XML files (*.xml)"));
    if((filename.isNull()) || (filename.isEmpty()) || (!QFile::exists(filename)))
    {
        qDebug() << "[ObjectImporter] Not able to find the selected file " << filename;
        return false;
    }

    QDomDocument doc("DMHelperXML");
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[ObjectImporter] Not able to open the selected file " << filename;
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString contentError;
    int contentErrorLine = 0;
    int contentErrorColumn = 0;
    bool contentResult = doc.setContent(in.readAll(), &contentError, &contentErrorLine, &contentErrorColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[ObjectImporter] Loading Failed: Error reading XML (line " << contentErrorLine << ", column " << contentErrorColumn << "): " << contentError;
        return false;
    }

    QDomElement rootObject = doc.documentElement();
    if((rootObject.isNull()) || (rootObject.tagName() != "root"))
    {
        qDebug() << "[ObjectImporter] Loading Failed: Error reading XML - unable to find root entry";
        return false;
    }

    QDomElement campaignElement = rootObject.firstChildElement(QString("campaign"));
    if(campaignElement.isNull())
    {
        qDebug() << "[ObjectImporter] Loading Failed: Error reading XML - unable to find campaign entry";
        return false;
    }

    QFileInfo fileInfo(filename);
    QDir::setCurrent(fileInfo.absolutePath());
    QScopedPointer<Campaign> importCampaign(new Campaign());
    importCampaign->inputXML(campaignElement, true);
    importCampaign->postProcessXML(campaignElement, true);
    if(!importCampaign->isValid())
    {
        QMessageBox::critical(nullptr,
                              QString("Invalid Campaign"),
                              QString("The imported file contains a campaign with an invalid structure."));
        return false;
    }

    int i;
    bool duplicateFree = true;
    QList<CampaignObjectBase*> childList = importCampaign->getChildObjects();
    for(i = 0; i < childList.count(); ++i)
    {
        duplicateFree = duplicateFree && checkObjectDuplicates(childList.at(i), campaign, *importCampaign);
    }

    if(!duplicateFree)
    {
        QString duplicateString("The imported campaign contains the following items with IDs that are already in the current campaign. These items will not be imported to avoid duplicate IDs in the campaign.");
        duplicateString += QChar::LineFeed + QChar::LineFeed;

        for(QString duplicate : _duplicateObjects)
        {
            duplicateString += duplicate + QChar::LineFeed;
        }

        QMessageBox::critical(nullptr, QString("Duplicate import IDs"), duplicateString);
    }

    childList = importCampaign->getChildObjects();
    for(i = 0; i < childList.count(); ++i)
    {
        campaign.addObject(childList.at(i));
    }

    return true;

    /*
    int i;

    // Check the party
    for(i = 0; i < importCampaign->getCharacterCount(); ++i)
    {
        Character* character = importCampaign->getCharacterByIndex(i);
        if(character)
        {
            if(campaign.getCharacterById(character->getID()) == nullptr)
                campaign.addCharacter(new Character(*character));
        }
    }

    // Check the settings
    for(i = 0; i < importCampaign->getSettingCount(); ++i)
    {
        Map* setting = importCampaign->getSettingByIndex(i);
        if(setting)
        {
            if(campaign.getSettingById(setting->getID()) == nullptr)
                campaign.addSetting(new Map(*setting));
        }
    }

    // Check the NPCs
    for(i = 0; i < importCampaign->getNPCCount(); ++i)
    {
        Character* npc = importCampaign->getNPCByIndex(i);
        if(npc)
        {
            if(campaign.getNPCById(npc->getID()) == nullptr)
                campaign.addNPC(new Character(*npc));
        }
    }

    // Check the audio tracks
    for(i = 0; i < importCampaign->getTrackCount(); ++i)
    {
        AudioTrack* track = importCampaign->getTrackByIndex(i);
        if(track)
        {
            if(campaign.getTrackById(track->getID()) == nullptr)
                campaign.addTrack(new AudioTrack(*track));
        }
    }

    // Iterate through the adventures and check them and their contents
    for(i = 0; i < importCampaign->getAdventureCount(); ++i)
    {
        Adventure* importAdventure = importCampaign->getAdventureByIndex(i);
        if(importAdventure)
        {
            Adventure* adventure = campaign.getAdventureById(importAdventure->getID());
            if(adventure == nullptr)
            {
                campaign.addAdventure(new Adventure(*importAdventure));
            }
            else
            {
                int j;
                for(j = 0; j < importAdventure->getEncounterCount(); ++j)
                {
                    Encounter* encounter = importAdventure->getEncounterByIndex(j);
                    if(encounter)
                    {
                        if(adventure->getEncounterById(encounter->getID()) == nullptr)
                            adventure->addEncounter(EncounterFactory::cloneEncounter(*encounter));
                    }
                }

                for(j = 0; j < importAdventure->getMapCount(); ++j)
                {
                    Map* map = importAdventure->getMapByIndex(j);
                    if(map)
                    {
                        if(adventure->getMapById(map->getID()) == nullptr)
                            adventure->addMap(new Map(*map));
                    }
                }
            }
        }
    }

    return true;
*/
    /*
    QDomElement baseElement = rootObject.firstChildElement();
    if(baseElement.tagName() == QString("combatant"))
    {
        return importCombatant(campaign, item, baseElement);
    }
    else if(baseElement.tagName() == QString("encounter"))
    {
        return importEncounter(campaign, item, baseElement);
    }
    else if(baseElement.tagName() == QString("map"))
    {
        return importMap(campaign, item, baseElement);
    }
    else if(baseElement.tagName() == QString("adventure"))
    {
        return importAdventure(campaign, item, baseElement);
    }

    qDebug() << "[ObjectImporter] Loading Failed: Unable to find a supported object for import";
    return false;
    */
}

bool ObjectImporter::checkObjectDuplicates(CampaignObjectBase* object, Campaign& targetCampaign, Campaign& importCampaign)
{
    if(targetCampaign.getObjectById(object->getID()) != nullptr)
    {
        _duplicateObjects.append(object->getName() + QString(": ") + object->getID().toString());
        importCampaign.removeObject(object->getID());
        object->deleteLater();
        return false;
    }

    QList<CampaignObjectBase*> childList = object->getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        if(!checkObjectDuplicates(childList.at(i), targetCampaign, importCampaign))
            return false;
    }

    return true;
}


/*
QUuid ObjectImporter::importCombatant(Campaign& campaign, QStandardItem* item, QDomElement& element)
{
    Character* newCharacter = new Character(element, true);
    if(isWorldEntry(item))
    {
        campaign.addNPC(newCharacter);
    }
    else
    {
        campaign.addCharacter(newCharacter);
    }

    newCharacter->postProcessXML(element, true);
    return newCharacter->getID();
}

QUuid ObjectImporter::importEncounter(Campaign& campaign, QStandardItem* item, QDomElement& element)
{
    bool ok = false;
    int encounterType = element.attribute("type").toInt(&ok);
    if(!ok)
        return QUuid();

    QStandardItem* adventureItem = findParentbyType(item, DMHelper::TreeType_Adventure);
    if(!adventureItem)
        return QUuid();

    Adventure* adventure = campaign.getAdventureById(QUuid(adventureItem->data(DMHelper::TreeItemData_ID).toString()));
    if(!adventure)
        return QUuid();

    Encounter* newEncounter = EncounterFactory::createEncounter(encounterType, element, true, adventure);
    if(!newEncounter)
        return QUuid();

    adventure->addEncounter(newEncounter);
    newEncounter->postProcessXML(element, true);

    return newEncounter->getID();
}

QUuid ObjectImporter::importMap(Campaign& campaign, QStandardItem* item, QDomElement& element)
{
    Map* newMap = new Map(element, true);
    QStandardItem* adventureItem = findParentbyType(item, DMHelper::TreeType_Adventure);
    Adventure* adventure = nullptr;
    if(adventureItem)
        adventure = campaign.getAdventureById(QUuid(adventureItem->data(DMHelper::TreeItemData_ID).toString()));

    if(adventure)
    {
        adventure->addMap(newMap);
    }
    else
    {
        campaign.addSetting(newMap);
    }

    newMap->postProcessXML(element, true);

    return newMap->getID();
}

QUuid ObjectImporter::importAdventure(Campaign& campaign, QStandardItem* item, QDomElement& element)
{
    Q_UNUSED(item);

    Adventure* newAdventure = new Adventure(element, true);
    campaign.addAdventure(newAdventure);
    newAdventure->postProcessXML(element, true);
    return newAdventure->getID();
}

bool ObjectImporter::isWorldEntry(QStandardItem* item)
{
    if(!item)
        return false;

    // Check if the item itself is a World type
    if(item->data(DMHelper::TreeItemData_Type).toInt() == DMHelper::TreeType_World)
        return true;

    // Check up the parent tree if there is a World type
    QStandardItem* parentItem = item->parent();
    while(parentItem)
    {
        if(parentItem->data(DMHelper::TreeItemData_Type).toInt() == DMHelper::TreeType_World)
            return true;
        parentItem = parentItem->parent();
    }

    return false;
}

QStandardItem* ObjectImporter::findParentbyType(QStandardItem* child, int parentType)
{
    QStandardItem* result = child;

    while(result)
    {
        if(result->data(DMHelper::TreeItemData_Type).toInt() == parentType)
            return result;

        result = result->parent();
    }

    return nullptr;
}
*/
