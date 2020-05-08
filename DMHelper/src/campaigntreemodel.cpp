#include "campaigntreemodel.h"
#include "combatant.h"
#include "character.h"
#include "campaigntreeitem.h"
#include "dmconstants.h"
#include <QMimeData>
#include <QUuid>
#include <QTimer>
#include <QDebug>

// Uncomment the next line to log in detail all of the additions to the campaign model
//#define CAMPAIGN_MODEL_LOGGING

CampaignTreeModel::CampaignTreeModel(QObject *parent) :
    QStandardItemModel(parent),
    _campaign(nullptr),
    _objectIndexMap(),
    _updateParent(nullptr),
    _updateRow(-1)
{
    setItemPrototype(new CampaignTreeItem());
    connect(this, &QStandardItemModel::itemChanged, this, &CampaignTreeModel::handleItemChanged);
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

CampaignTreeItem *CampaignTreeModel::campaignItem(int row, int column) const
{
    return dynamic_cast<CampaignTreeItem*>(item(row, column));
}

CampaignTreeItem *CampaignTreeModel::campaignItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CampaignTreeItem*>(itemFromIndex(index));
}

QMimeData *	CampaignTreeModel::mimeData(const QModelIndexList & indexes) const
{
    QMimeData *data = QStandardItemModel::mimeData(indexes);
    if(!data)
        return nullptr;

    if(indexes.count() == 1)
    {
        CampaignTreeItem* item = campaignItemFromIndex(indexes.at(0));
        if(item)
        {
            QByteArray encodedData;
            QDataStream stream(&encodedData, QIODevice::WriteOnly);
            //stream << item->data(DMHelper::TreeItemData_Type).toInt() << QUuid(item->data(DMHelper::TreeItemData_ID).toString());
            quint64 ptr = reinterpret_cast<quint64>(item->getCampaignItemObject());
            stream << item->getCampaignItemType() << item->getCampaignItemId() << ptr;
            data->setData(QString("application/vnd.dmhelper.text"), encodedData);
        }
    }

    return data;
}

QStringList	CampaignTreeModel::mimeTypes() const
{
    return QStandardItemModel::mimeTypes() <<  QLatin1String("application/vnd.dmhelper.text");
}

bool CampaignTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    bool result = QStandardItemModel::dropMimeData(data, action, row, column, parent);

    if((!data) || (!parent.isValid()))
        return result;

    QByteArray encodedData = data->data(QString("application/vnd.dmhelper.text"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int treeItemDataType;
    QUuid treeItemDataID;
    quint64 treeItemObjectPtr;
    stream >> treeItemDataType >> treeItemDataID >> treeItemObjectPtr;

    // TODO: Clean up, probably don't even need the MimeData!
    CampaignObjectBase* movedObject = reinterpret_cast<CampaignObjectBase*>(treeItemObjectPtr);
    CampaignTreeItem* parentItem = campaignItemFromIndex(parent);
    CampaignObjectBase* parentObject = nullptr;
    if(parentItem)
        parentObject = parentItem->getCampaignItemObject();
    //QModelIndex insertedIndex = index(row, column, parent);
    //CampaignTreeItem* insertedItem = campaignItemFromIndex(insertedIndex);
    if((row < 0) || (row > rowCount(parent)))
        row = rowCount(parent);
    QStandardItem* insertedStandardItem = parentItem->child(row - 1, 0);
    CampaignTreeItem* insertedItem = dynamic_cast<CampaignTreeItem*>(insertedStandardItem);
    CampaignObjectBase* insertedObject = nullptr;
    if(insertedItem)
        insertedObject = insertedItem->getCampaignItemObject();

    // Open the item's new parent
    if(parentObject)
        parentObject->setExpanded(true);

    // Set the campaign parent for the item
    validateIndividualChild(parentItem, insertedItem);

    // Update the moved object's icon if needed
    iterateTreeEntryVisualization(insertedItem);

    emit itemMoved(insertedItem);

    return result;
}


void CampaignTreeModel::setCampaign(Campaign* campaign)
{
    if(!campaign)
    {
        _campaign = nullptr;
        clear();
    }
    else if(_campaign != campaign)
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

void CampaignTreeModel::handleRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(last);
    //QModelIndex insertedIndex = index(first, 0, parent);
    //QStandardItem* insertedItem = itemFromIndex(insertedIndex);
    QStandardItem* parentItem = itemFromIndex(parent);
    //QStandardItem* insertedItem = parentItem->child(first);
    //qDebug() << "[CampaignTreeModel] Row inserted: " << parent << "=" << (parentItem ? parentItem->text() : QString("null")) << ", first: " << first << ", last: " << last << ": " << (insertedItem ? insertedItem->text() : QString("null"));

    /*
    QModelIndex insertedIndex = index(first, 0, parent);
    CampaignObjectBase* insertedObject = reinterpret_cast<CampaignObjectBase*>(data(insertedIndex, DMHelper::TreeItemData_Object).value<quint64>());
    QUuid insertedId = data(insertedIndex, DMHelper::TreeItemData_ID).toString();
    QVariant displayRole = data(insertedIndex);
    */

    _updateParent = parentItem;
    _updateRow = first;
    //QTimer::singleShot(0, this, SLOT(handleTimer()));
}

/*
void CampaignTreeModel::handleRowsRemoved(const QModelIndex &parent, int first, int last)
{
    QModelIndex insertedIndex = index(first, 0, parent);
    QStandardItem* insertedItem = itemFromIndex(insertedIndex);
    QStandardItem* parentItem = itemFromIndex(parent);
    qDebug() << "[CampaignTreeModel] Row removed: " << parent << "=" << (parentItem ? parentItem->text() : QString("null")) << ", first: " << first << ", last: " << last << ": " << insertedIndex << "=" << (insertedItem ? insertedItem->text() : QString("null"));
}
*/

void CampaignTreeModel::handleTimer()
{
    validateIndividualChild(_updateParent, _updateRow);
    //emit itemMoved(_updateParent, _updateRow);
    _updateParent = nullptr;
    _updateRow = -1;
}

void CampaignTreeModel::handleItemChanged(QStandardItem *item)
{
    CampaignTreeItem* campaignItem = dynamic_cast<CampaignTreeItem*>(item);
    if((campaignItem) && (campaignItem->getCampaignItemType() == DMHelper::CampaignType_Combatant))
    {
        Character* character = dynamic_cast<Character*>(campaignItem->getCampaignItemObject());
        if(character)
            character->setActive(item->checkState() == Qt::Checked);
    }
}

void CampaignTreeModel::updateCampaignEntries()
{
    if(!_campaign)
        return;

    qDebug() << "[CampaignTreeModel] Campaign update detected, recreating campaign model.";

    disconnect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(handleRowsInserted(const QModelIndex &, int, int)));
    // disconnect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(handleRowsRemoved(const QModelIndex &, int, int)));

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

    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(handleRowsInserted(const QModelIndex &, int, int)));
    //connect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(handleRowsRemoved(const QModelIndex &, int, int)));
}

QStandardItem* CampaignTreeModel::createTreeEntry(CampaignObjectBase* object, QStandardItem* parentEntry)
{
    if(!object)
        return nullptr;

    //QStandardItem* treeEntry = new QStandardItem(object->getName());
    CampaignTreeItem* treeEntry = new CampaignTreeItem(object->getName());
    treeEntry->setEditable(true);

//    treeEntry->setData(QVariant::fromValue(static_cast<void*>(object)),DMHelper::TreeItemData_Object);
    treeEntry->setData(QVariant::fromValue(reinterpret_cast<quint64>(object)), DMHelper::TreeItemData_Object);
    //campaignItem->setData(QVariant(DMHelper::TreeType_Campaign),DMHelper::TreeItemData_Type);
    treeEntry->setData(QVariant(object->getObjectType()), DMHelper::TreeItemData_Type);
    //campaignItem->setData(QVariant(QString()),DMHelper::TreeItemData_ID);
    treeEntry->setData(QVariant(object->getID().toString()), DMHelper::TreeItemData_ID);
    //treeModel->appendRow(campaignItem);
    //ui->treeView->expand(campaignItem->index());

    setTreeEntryVisualization(treeEntry);

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

#ifdef CAMPAIGN_MODEL_LOGGING
    qDebug() << "[CampaignTreeModel] Added object: " << objectEntry->text() << ", ID: " << objectId << ", Index: " << objectEntry->index();
#endif
}

void CampaignTreeModel::validateChildStructure(QStandardItem* parentItem)
{
    if(!parentItem)
        return;

    CampaignObjectBase* parentObject = reinterpret_cast<CampaignObjectBase*>(parentItem->data(DMHelper::TreeItemData_Object).value<quint64>());
    QUuid parentId = parentItem->data(DMHelper::TreeItemData_ID).toString();
    if((!parentObject) || (parentId.isNull()))
    {
        qDebug() << "[CampaignTreeModel] ERROR: Not possible to validate child structure - parent ID: " << parentId << ", object: " << parentObject;
        return;
    }

    for(int i = 0; i < parentItem->rowCount(); ++i)
    {
        QStandardItem* childItem = parentItem->child(i);
        if(childItem)
        {
            CampaignObjectBase* childObject = reinterpret_cast<CampaignObjectBase*>(childItem->data(DMHelper::TreeItemData_Object).value<quint64>());
            if(childObject)
            {
                const CampaignObjectBase* currentParentObject = qobject_cast<const CampaignObjectBase*>(childObject->parent());
                if((!currentParentObject) || (currentParentObject->getID() != parentId))
                    childObject->setParent(parentObject);
            }
        }
    }
}

void CampaignTreeModel::validateIndividualChild(QStandardItem* parentItem, int row)
{
    if((!parentItem) || (row < 0) || (row >= parentItem->rowCount()))
        return;

    validateIndividualChild(parentItem, parentItem->child(row));
}

void CampaignTreeModel::validateIndividualChild(QStandardItem* parentItem, QStandardItem* childItem)
{
    if((!parentItem) || (!childItem))
        return;

    CampaignObjectBase* parentObject = reinterpret_cast<CampaignObjectBase*>(parentItem->data(DMHelper::TreeItemData_Object).value<quint64>());
    QUuid parentId = parentItem->data(DMHelper::TreeItemData_ID).toString();
    if((!parentObject) || (parentId.isNull()))
    {
        qDebug() << "[CampaignTreeModel] ERROR: Not possible to validate child  - parent ID: " << parentId << ", object: " << parentObject;
        return;
    }

    CampaignObjectBase* childObject = reinterpret_cast<CampaignObjectBase*>(childItem->data(DMHelper::TreeItemData_Object).value<quint64>());
    if(childObject)
    {
        const CampaignObjectBase* currentParentObject = qobject_cast<const CampaignObjectBase*>(childObject->parent());
        if((!currentParentObject) || (currentParentObject->getID() != parentId))
            childObject->setParent(parentObject);
    }
}

void CampaignTreeModel::iterateTreeEntryVisualization(CampaignTreeItem* entry)
{
    if(!entry)
        return;

    setTreeEntryVisualization(entry);

    for(int i = 0; i < entry->rowCount(); ++i)
    {
        iterateTreeEntryVisualization(dynamic_cast<CampaignTreeItem*>(entry->child(i)));
    }
}

void CampaignTreeModel::setTreeEntryVisualization(CampaignTreeItem* entry)
{
    if(!entry)
        return;

    CampaignObjectBase* object = entry->getCampaignItemObject();
    if(!object)
        return;

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            entry->setIcon(QIcon(":/img/data/icon_contentparty.png"));
            break;
        case DMHelper::CampaignType_Combatant:
            {
                Character* character = dynamic_cast<Character*>(object);
                bool isPC = ((character) && (character->isInParty()));
                entry->setIcon(isPC ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png"));
                entry->setEditable(false);
                entry->setCheckable(isPC);
                if(isPC)
                    entry->setCheckState(character->getActive() ? Qt::Checked : Qt::Unchecked);
                else
                    entry->setData(QVariant(), Qt::CheckStateRole); // Needed to actively remove the checkbox on the entry
            }
            break;
        /*
            {
                Combatant* combatant = dynamic_cast<Combatant*>(object);
                if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Character))
                {
                    Character* character = dynamic_cast<Character*>(combatant);
                    if((character) && (character->isInParty()))
                    {
                        treeEntry->setCheckable(true);
                        treeEntry->setIcon(QIcon(":/img/data/icon_character.png"));
                        break;
                    }
                }
            }
            treeEntry->setIcon(QIcon(":/img/data/icon_contentnpc.png"));
            break;
        */
        case DMHelper::CampaignType_Map:
            entry->setIcon(QIcon(":/img/data/icon_contentmap.png"));
            break;
        case DMHelper::CampaignType_Text:
            entry->setIcon(QIcon(":/img/data/icon_contenttextencounter.png"));
            break;
        case DMHelper::CampaignType_Battle:
            entry->setIcon(QIcon(":/img/data/icon_contentbattle.png"));
            break;
        case DMHelper::CampaignType_ScrollingText:
            entry->setIcon(QIcon(":/img/data/icon_contentscrollingtext.png"));
            break;
        case DMHelper::CampaignType_AudioTrack:
            entry->setIcon(QIcon(":/img/data/icon_music.png"));
            break;
    }

}
