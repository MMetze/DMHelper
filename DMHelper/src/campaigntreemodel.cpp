#include "campaigntreemodel.h"
#include "combatant.h"
#include "character.h"
#include "map.h"
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

QModelIndex CampaignTreeModel::getObjectIndex(const QUuid& objectId) const
{
    qDebug() << "[CampaignTreeModel] Searching for index for object id: " << objectId;
    QModelIndex result = _objectIndexMap.value(objectId);
    if(!result.isValid())
        qDebug() << "[CampaignTreeModel] ERROR: Not able to find a valid index for object id: " << objectId;

    qDebug() << "[CampaignTreeModel]     result: " << result << ", valid: " << result.isValid();
    return result;
}

CampaignTreeItem* CampaignTreeModel::getObjectItem(const QUuid& objectId) const
{
    qDebug() << "[CampaignTreeModel] Searching for item for object id: " << objectId;
    QModelIndex index = getObjectIndex(objectId);
    if(!index.isValid())
        return nullptr;

    CampaignTreeItem* item = campaignItemFromIndex(index);
    if((item) && (item->getCampaignItemId() != objectId))
    {
        qDebug() << "[CampaignTreeModel] ERROR: Found item ID does not match: no object found!";
        return nullptr;
    }

    return item;
}

CampaignTreeItem *CampaignTreeModel::campaignItem(int row, int column) const
{
    return dynamic_cast<CampaignTreeItem*>(item(row, column));
}

CampaignTreeItem *CampaignTreeModel::campaignItemFromIndex(const QModelIndex &index) const
{
    return dynamic_cast<CampaignTreeItem*>(itemFromIndex(index));
}

QMimeData* CampaignTreeModel::mimeData(const QModelIndexList & indexes) const
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

QStringList CampaignTreeModel::mimeTypes() const
{
    return QStandardItemModel::mimeTypes() <<  QLatin1String("application/vnd.dmhelper.text");
}

bool CampaignTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    bool result = QStandardItemModel::dropMimeData(data, action, row, column, parent);

    //if((!data) || (!parent.isValid()))
    if(!data)
    {
        qDebug() << "[CampaignTreeModel] ERROR: drop not properly completed, empty mime data: " << data; // << " or invalid parent index: " << parent;
        return result;
    }

    QByteArray encodedData = data->data(QString("application/vnd.dmhelper.text"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int treeItemDataType;
    QUuid treeItemDataID;
    quint64 treeItemObjectPtr;
    stream >> treeItemDataType >> treeItemDataID >> treeItemObjectPtr;

    //CampaignTreeItem* parentItem = campaignItemFromIndex(parent);
    QStandardItem* parentItem = parent.isValid() ? itemFromIndex(parent) : invisibleRootItem();
    if(!parentItem)
    {
        qDebug() << "[CampaignTreeModel] ERROR: drop not properly completed, unable to find parent item for index: " << parent;
        return result;
    }

    // This will only happen if the parent is not the root item
    CampaignTreeItem* parentCampaignItem = dynamic_cast<CampaignTreeItem*>(parentItem);
    if(parentCampaignItem)
    {
        CampaignObjectBase* parentObject = parentCampaignItem->getCampaignItemObject();
        // Open the item's new parent
        if(parentObject)
            parentObject->setExpanded(true);
    }

    qDebug() << "[CampaignTreeModel] object ID " << treeItemDataID << " dropped on parent item: " << parentItem->text() << " with rows: " << parentItem->rowCount() << " in row = " << row;
    CampaignTreeItem* insertedItem = getChildById(parentItem, treeItemDataID);
    if(!insertedItem)
    {
        qDebug() << "[CampaignTreeModel] ERROR: drop not properly completed, unable to find child item for id: " << treeItemDataID << " within parent item " << parentItem << " from parent index " << parent;
        return result;
    }

    // Set the campaign parent for the item
    validateIndividualChild(parentItem, insertedItem);

    // Update the moved object's icon if needed
    iterateTreeEntryVisualization(insertedItem);

    // Update the row IDs in the children
    updateChildRows(parentItem);

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
    qDebug() << "[CampaignTreeModel] Row inserted: " << parent << "=" << (parentItem ? parentItem->text() : QString("null")) << ", first: " << first << ", last: " << last; // << ": " << (insertedItem ? insertedItem->text() : QString("null"));
    updateChildRows(parentItem ? parentItem : invisibleRootItem());

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

void CampaignTreeModel::handleRowsRemoved(const QModelIndex &parent, int first, int last)
{
    /*
    QModelIndex insertedIndex = index(first, 0, parent);
    QStandardItem* insertedItem = itemFromIndex(insertedIndex);
    */
    QStandardItem* parentItem = itemFromIndex(parent);
    qDebug() << "[CampaignTreeModel] Row removed: " << parent << "=" << (parentItem ? parentItem->text() : QString("null")) << ", first: " << first << ", last: " << last; // << ": " << insertedIndex << "=" << (insertedItem ? insertedItem->text() : QString("null"));
    updateChildRows(parentItem ? parentItem : invisibleRootItem());
}

void CampaignTreeModel::handleRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    QStandardItem* parentItem = itemFromIndex(parent);
    QStandardItem* destinationItem = itemFromIndex(destination);
    qDebug() << "[CampaignTreeModel] Row moved from: " << parent << "=" << (parentItem ? parentItem->text() : QString("null")) << ", start: " << start << ", end: " << end << " to: " << destination << "=" << (destinationItem ? destinationItem->text() : QString("null")) << ", row: " << row; // << ": " << insertedIndex << "=" << (insertedItem ? insertedItem->text() : QString("null"));
    updateChildRows(parentItem ? parentItem : invisibleRootItem());
}

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

void CampaignTreeModel::handleObjectNameChanged(CampaignObjectBase* object, const QString& name)
{
    if(!object)
        return;

    qDebug() << "[CampaignTreeModel] Object " << object << " has a new name " << name;

    QUuid objId = object->getID();
    CampaignTreeItem* item = getObjectItem(objId);
    //CampaignTreeItem* item = campaignItemFromIndex(getObject(object->getID()));
    qDebug() << "[CampaignTreeModel] ObjId: " << objId << ", item: " << item;
    if(!item)
    {
        qDebug() << "[CampaignTreeModel] Unable to find tree item for object " << object;
        return;
    }

    qDebug() << "[CampaignTreeModel] Item " << item << ", item ID: " << item->getCampaignItemId() << " object ID: " << object->getID() << ", name: " << item->text();

    if(item->text() != name)
    {
        item->setText(name);
    }
}

void CampaignTreeModel::updateCampaignEntries()
{
    if(!_campaign)
        return;

    qDebug() << "[CampaignTreeModel] Campaign update detected, recreating campaign model.";

    disconnect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(handleRowsInserted(const QModelIndex &, int, int)));
    //disconnect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(handleRowsRemoved(const QModelIndex &, int, int)));
    disconnect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(handleRowsRemoved(const QModelIndex &, int, int)));
    disconnect(this, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)), this, SLOT(handleRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));

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

    updateChildRows(invisibleRootItem());

    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(handleRowsInserted(const QModelIndex &, int, int)));
    //connect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(handleRowsRemoved(const QModelIndex &, int, int)));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(handleRowsRemoved(const QModelIndex &, int, int)));
    connect(this, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)), this, SLOT(handleRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
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

    if(object->getObjectType() == DMHelper::CampaignType_Map)
    {
        Map* mapObject = dynamic_cast<Map*>(object);
        treeEntry->setToolTip(mapObject->getFileName());
    }

    connect(object, &CampaignObjectBase::nameChanged, this, &CampaignTreeModel::handleObjectNameChanged);

    setTreeEntryVisualization(treeEntry);

    addTreeEntry(treeEntry, parentEntry);

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

    updateChildRows(treeEntry);

    return treeEntry;
}

void CampaignTreeModel::addTreeEntry(CampaignTreeItem* objectEntry, QStandardItem* parentEntry)
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

    int targetRow = 0;
    int objectEntryRow;
    int childEntryRow;
    CampaignTreeItem* childEntry = nullptr;
    if(objectEntry->getCampaignItemRow() != -1)
    {
        objectEntryRow = objectEntry->getCampaignItemRow();
        childEntry = dynamic_cast<CampaignTreeItem*>(parentEntry->child(targetRow));
        if(childEntry)
            childEntryRow = childEntry->getCampaignItemRow();
        while((childEntry != nullptr) && (childEntry->getCampaignItemRow() <= objectEntry->getCampaignItemRow()))
        {
            childEntry = dynamic_cast<CampaignTreeItem*>(parentEntry->child(++targetRow));
        }
    }

    if(childEntry == nullptr)
        parentEntry->appendRow(objectEntry);
    else
        parentEntry->insertRow(targetRow, objectEntry);

    _objectIndexMap.insert(objectId, QPersistentModelIndex(objectEntry->index()));

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

//    QUuid parentId;
    CampaignObjectBase* parentObject = reinterpret_cast<CampaignObjectBase*>(parentItem->data(DMHelper::TreeItemData_Object).value<quint64>());
    if(!parentObject)
    {
        parentObject = _campaign;
//        parentId = QUuid(parentItem->data(DMHelper::TreeItemData_ID).toString());
    }
//    else
//    {
//        parentId = QUuid(parentItem->data(DMHelper::TreeItemData_ID).toString());
//    }

    if(!parentObject)
    {
        qDebug() << "[CampaignTreeModel] ERROR: Not possible to validate child  - parent object: " << parentObject;
        return;
    }

    QUuid parentId = parentObject->getID();

    if(parentId.isNull())
    {
        qDebug() << "[CampaignTreeModel] ERROR: Not possible to validate child  - parent ID: " << parentId;
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
                //entry->setEditable(false);
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

void CampaignTreeModel::updateChildRows(QStandardItem* parentItem)
{
    for(int i = 0; i < parentItem->rowCount(); ++i)
    {
        CampaignTreeItem* childItem = dynamic_cast<CampaignTreeItem*>(parentItem->child(i));
        if(childItem)
            childItem->setCampaignItemRow(i);
    }
}

CampaignTreeItem* CampaignTreeModel::getChildById(QStandardItem* parentItem, const QUuid& itemId) const
{
    if(!parentItem)
        return nullptr;

    for(int i = 0; i < parentItem->rowCount(); ++i)
    {
        CampaignTreeItem* item = dynamic_cast<CampaignTreeItem*>(parentItem->child(i));
        if(item)
        {
            if(item->getCampaignItemId() == itemId)
                return item;

            CampaignTreeItem* childItem = item->getChildById(itemId);
            if(childItem)
                return childItem;
        }
    }

    return nullptr;
}

/*
void CampaignTreeModel::rebuildIndexMap()
{
    if(!_campaign)
        return;

    _objectIndexMap.clear();
    clear();

    QList<CampaignObjectBase*> childObjects = _campaign->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        _objectIndexMap.insert(objectId, objectEntry->index());
    }
}
*/
