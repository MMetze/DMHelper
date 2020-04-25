#include "campaigntreemodel.h"
#include "combatant.h"
#include "dmconstants.h"
#include <QMimeData>
#include <QUuid>
#include <QDebug>

CampaignTreeModel::CampaignTreeModel(QObject *parent) :
    QStandardItemModel(parent),
    _campaign(nullptr),
    _objectIndexMap()
{
}

Campaign* CampaignTreeModel::getCampaign() const
{
    return _campaign;
}

bool CampaignTreeModel::containsObject(const QUuid& objectId) const
{
    return _objectIndexMap.contains(objectId);
}

QModelIndex CampaignTreeModel::getObject(const QUuid& objectId) const
{
    qDebug() << "[CampaignTreeModel] Searching for object id: " << objectId;
    QModelIndex result = _objectIndexMap.value(objectId);
    qDebug() << "[CampaignTreeModel]     result: " << result;
    return result;
}

QMimeData *	CampaignTreeModel::mimeData(const QModelIndexList & indexes) const
{
    QMimeData *data = QStandardItemModel::mimeData(indexes);
    if(!data)
        return nullptr;

    if(indexes.count() == 1)
    {
        QStandardItem* item = itemFromIndex(indexes.at(0));
        if(item)
        {
            QByteArray encodedData;
            QDataStream stream(&encodedData, QIODevice::WriteOnly);
            stream << item->data(DMHelper::TreeItemData_Type).toInt() << QUuid(item->data(DMHelper::TreeItemData_ID).toString());
            data->setData(QString("application/vnd.dmhelper.text"), encodedData);
        }
    }

    return data;
}

QStringList	CampaignTreeModel::mimeTypes() const
{
    return QStandardItemModel::mimeTypes() <<  QLatin1String("application/vnd.dmhelper.text");
}

void CampaignTreeModel::setCampaign(Campaign* campaign)
{
    if(_campaign != campaign)
    {
        _campaign = campaign;

        updateCampaignEntries();

        emit campaignChanged(_campaign);
    }
}

void CampaignTreeModel::refresh()
{
    if(!_campaign)
        return;

    updateCampaignEntries();
    emit campaignChanged(_campaign);
}

void CampaignTreeModel::updateCampaignEntries()
{
    if(!_campaign)
        return;

    qDebug() << "[CampaignTreeModel] Campaign update detected, recreating campaign model.";

    _objectIndexMap.clear();
    clear();

    QList<CampaignObjectBase*> childObjects = _campaign->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        createTreeEntry(childObject, invisibleRootItem());
        /*
        QStandardItem* objectEntry = createTreeEntry(childObject);
        if(objectEntry)
        {
            appendRow(objectEntry);
        }
        */
    }
}

QStandardItem* CampaignTreeModel::createTreeEntry(CampaignObjectBase* object, QStandardItem* parentEntry)
{
    if(!object)
        return nullptr;

    QStandardItem* treeEntry = new QStandardItem(object->getName());
    treeEntry->setEditable(true);

//    treeEntry->setData(QVariant::fromValue(static_cast<void*>(object)),DMHelper::TreeItemData_Object);
    treeEntry->setData(QVariant::fromValue(reinterpret_cast<uintptr_t>(object)),DMHelper::TreeItemData_Object);
    //campaignItem->setData(QVariant(DMHelper::TreeType_Campaign),DMHelper::TreeItemData_Type);
    //campaignItem->setData(QVariant(QString()),DMHelper::TreeItemData_ID);
    treeEntry->setData(QVariant(object->getID().toString()),DMHelper::TreeItemData_ID);
    //treeModel->appendRow(campaignItem);
    //ui->treeView->expand(campaignItem->index());

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            treeEntry->setIcon(QIcon(":/img/data/icon_newadventure.png"));
            break;
        case DMHelper::CampaignType_Combatant:
            {
                Combatant* combatant = dynamic_cast<Combatant*>(object);
                if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Character))
                {
                    treeEntry->setCheckable(true);
                }
            }
            treeEntry->setIcon(QIcon(":/img/data/icon_newcharacter.png"));
            break;
        case DMHelper::CampaignType_Map:
            treeEntry->setIcon(QIcon(":/img/data/icon_newmap.png"));
            break;
        case DMHelper::CampaignType_Text:
            treeEntry->setIcon(QIcon(":/img/data/icon_newtextencounter.png"));
            break;
        case DMHelper::CampaignType_Battle:
            treeEntry->setIcon(QIcon(":/img/data/icon_newbattle.png"));
            break;
        case DMHelper::CampaignType_ScrollingText:
            treeEntry->setIcon(QIcon(":/img/data/icon_newscrollingtext.png"));
            break;
        case DMHelper::CampaignType_AudioTrack:
            treeEntry->setIcon(QIcon(":/img/data/icon_music.png"));
            break;
    }

    appendTreeEntry(treeEntry, parentEntry);

    QList<CampaignObjectBase*> childObjects = object->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        createTreeEntry(childObject, treeEntry);
        /*
        QStandardItem* objectEntry = createTreeEntry(childObject);
        if(objectEntry)
            treeEntry->appendRow(objectEntry);
            */
    }

    return treeEntry;
}

void CampaignTreeModel::appendTreeEntry(QStandardItem* objectEntry, QStandardItem* parentEntry)
{
    if((!objectEntry) || (!parentEntry))
    {
        qDebug() << "[CampaignTreeModel] ERROR: unable to add entry to tree. Entry: " << objectEntry << ", Parent: " << parentEntry;
        return;
    }

    QUuid objectId(objectEntry->data(DMHelper::TreeItemData_ID).toString());
    if(objectId.isNull())
    {
        qDebug() << "[CampaignTreeModel] ERROR: unable to add entry to tree due to invalid ID. Entry: " << objectEntry << ", ID: " << objectId.toString();
        return;
    }

    parentEntry->appendRow(objectEntry);
    _objectIndexMap.insert(objectId, objectEntry->index());

    qDebug() << "[CampaignTreeModel] Added object: " << objectEntry->text() << ", ID: " << objectId << ", Index: " << objectEntry->index();
}
