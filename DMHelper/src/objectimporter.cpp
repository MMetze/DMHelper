#include "objectimporter.h"
#include "campaign.h"
#include "dmconstants.h"
#include "character.h"
#include "encounter.h"
#include "encounterfactory.h"
#include "map.h"
#include "adventure.h"
#include <QDomDocument>
#include <QFileDialog>
#include <QStandardItem>
#include <QTextStream>
#include <QDebug>

ObjectImporter::ObjectImporter(QObject *parent) :
    QObject(parent)
{
}

QUuid ObjectImporter::importObject(Campaign& campaign, QStandardItem* item)
{
    QString filename = QFileDialog::getOpenFileName(nullptr,QString("Select Campaign"));
    if((filename.isNull()) || (filename.isEmpty()) || (!QFile::exists(filename)))
    {
        qDebug() << "[ObjectImporter] Not able to find the selected file " << filename;
        return QUuid();
    }

    QDomDocument doc("DMHelperXML");
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[ObjectImporter] Not able to open the selected file " << filename;
        return QUuid();
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
        return QUuid();
    }

    QDomElement rootObject = doc.documentElement();
    if((rootObject.isNull()) || (rootObject.tagName() != "object"))
    {
        qDebug() << "[ObjectImporter] Loading Failed: Error reading XML - unable to find root object entry";
        return QUuid();
    }

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
    return QUuid();
}

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
