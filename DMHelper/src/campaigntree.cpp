#include "campaigntree.h"
#include "dmconstants.h"
#include "campaigntreemodel.h"
#include "campaigntreeitem.h"
#include "campaignobjectbase.h"
#include <QDropEvent>
#include <QStandardItemModel>
#include <QMimeData>
#include <QDebug>

CampaignTree::CampaignTree(QWidget *parent) :
    QTreeView(parent)
{
}

CampaignObjectBase* CampaignTree::currentCampaignObject()
{
    CampaignTreeModel* campaignModel = dynamic_cast<CampaignTreeModel*>(model());
    if(!campaignModel)
        return nullptr;

    QStandardItem* item = campaignModel->itemFromIndex(currentIndex());
    if(!item)
        return nullptr;

//    return static_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<void*>());
    return reinterpret_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<quint64>());
}

void CampaignTree::campaignChanged()
{
    updateExpandedState();
}

/*
void CampaignTree::handleItemMoved(QStandardItem* parentItem, int row)
{
    if(!parentItem)
        return;

    //CampaignObjectBase* parentObject = reinterpret_cast<CampaignObjectBase*>(parentItem->data(DMHelper::TreeItemData_Object).value<quint64>());
    //if(parentObject)
    //    parentObject->setExpanded(true);

    // Open the item's new parent
    //setExpanded(parentItem->index(), true);

    QStandardItem* childItem = parentItem->child(row);
    if(!childItem)
        return;

    // Set the expanded state of the item's children appropriately
    iterateItemExpanded(childItem);

    // Select the moved item
    setCurrentIndex(childItem->index());
}
*/

void CampaignTree::handleItemMoved(QStandardItem* movedItem)
{
    if(!movedItem)
        return;

    QModelIndex movedIndex = movedItem->index();

    // Open the item's new parent
    QModelIndex parentIndex = movedIndex.parent();
    if(parentIndex.isValid())
        setExpanded(parentIndex, true);

    // Set the expanded state of the item's children appropriately
    iterateItemExpanded(movedItem);

    // Select the moved item --> this really messes with the model, I assume the movement causes other items to get selected afterwards. If required to select the moved item, it needs to use a 0-duration timer
    //setCurrentIndex(movedIndex);
}

void CampaignTree::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeView::dragMoveEvent(event);

    if (!event->isAccepted())
        return;

    /*
    if((dropIndicatorPosition() == AboveItem) || (dropIndicatorPosition() == BelowItem))
        event->accept();
    else
        event->ignore(); //Show 'forbidden' cursor.
        */
    event->accept();
}

void CampaignTree::dropEvent(QDropEvent * event)
{
    /*
    event->ignore();

    const QMimeData* data = event->mimeData();
    QByteArray encodedData = data->data(QString("application/vnd.dmhelper.text"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int treeItemDataType;
    QUuid treeItemDataID;
    quint64 treeItemObjectPtr;
    stream >> treeItemDataType >> treeItemDataID >> treeItemObjectPtr;

    CampaignObjectBase* movedObject = reinterpret_cast<CampaignObjectBase*>(treeItemObjectPtr);
    if(!movedObject)
    {
        qDebug() << "[CampaignTree] ERROR: no base object found in drag data!";
        return;
    }

    QModelIndex targetIndex = indexAt(event->pos());
    if(!targetIndex.isValid())
        return;

    CampaignTreeModel* campaignModel = dynamic_cast<CampaignTreeModel*>(model());
    if(!campaignModel)
    {
        qDebug() << "[CampaignTree] ERROR: no model found for campaign tree, aborting drop";
        return;
    }

    CampaignTreeItem* targetItem = campaignModel->campaignItemFromIndex(targetIndex);
    if(!targetItem)
    {
        qDebug() << "[CampaignTree] ERROR: not able to find standard item for target index: " << targetIndex;
        return;
    }

    CampaignObjectBase* targetObject = targetItem->getCampaignItemObject();
    if(!targetItem)
    {
        qDebug() << "[CampaignTree] ERROR: not able to find campaign object item for target item: " << targetItem;
        return;
    }

    // Realign the moved object's parent
    CampaignObjectBase* parentObject = dynamic_cast<CampaignObjectBase*>(movedObject->parent());
    if((!parentObject) || (parentObject->getID() != targetObject->getID()))
        movedObject->setParent(targetObject);

    // Open the item's new parent
    targetObject->setExpanded(true);
    setExpanded(targetIndex, true);
    */

    QTreeView::dropEvent(event);

    /*
    event->ignore();

    const QMimeData* data = event->mimeData();
    QByteArray encodedData = data->data(QString("application/vnd.dmhelper.text"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int treeItemDataType;
    QUuid treeItemDataID;
    stream >> treeItemDataType >> treeItemDataID;

    QModelIndex index = indexAt(event->pos());
    if(!index.isValid())
        return;

    CampaignTreeModel* campaignModel = dynamic_cast<CampaignTreeModel*>(model());
    if(!campaignModel)
    {
        qDebug() << "[CampaignTree] ERROR: no model found for campaign tree, aborting drop";
        return;
    }

    Campaign* campaign = campaignModel->getCampaign();
    if(!campaign)
    {
        qDebug() << "[CampaignTree] ERROR: no active campaign found for campaign tree, aborting drop";
        return;
    }

    QStandardItem* item = campaignModel->itemFromIndex(index);
    if(!item)
    {
        qDebug() << "[CampaignTree] ERROR: not able to find standard item for target index: " << index;
        return;
    }

    CampaignObjectBase* movedObject =  campaign->getObjectById(treeItemDataID);
    if(!movedObject)
    {
        qDebug() << "[CampaignTree] ERROR: not able to find campaign object for the moved item with ID: " << treeItemDataID << ", type: " << treeItemDataType;
        return;
    }

    QTreeView::dropEvent(event);

    CampaignObjectBase* parentObject = dynamic_cast<CampaignObjectBase*>(movedObject->parent());
    if(parentObject)
    {
        qDebug() << "[CampaignTree] Drop identified" << movedObject->getName() << "(" << ((void*)movedObject) << "), ID " << movedObject->getID() << " to parent object " << parentObject->getName() << "(" << ((void*)parentObject) << "), ID " << parentObject->getID();
    }
    */

    /*
    CampaignObjectBase* parentObject = dynamic_cast<CampaignObjectBase*>(movedObject->parent());
    if(parentObject)
    {
        qDebug() << "[CampaignTree] Moving object " << movedObject->getName() << ", ID " << movedObject->getID() << " to parent object " << parentObject->getName() << ", ID " << parentObject->getID();
        parentObject->addObject(movedObject);
        parentObject->setExpanded(true);
    }
    else
    {
        qDebug() << "[CampaignTree] Moving object " << movedObject->getName() << ", ID " << movedObject->getID() << " to campaign object";
        campaign->addObject(movedObject);
    }
    */

    // Update the expand state of the tree items
    //updateExpandedState();


    /*
    int itemType = item->data(DMHelper::TreeItemData_Type).toInt();
    if( (itemType != treeItemDataType) ||
        ( ( itemType != DMHelper::TreeType_Encounter ) &&
          ( itemType != DMHelper::TreeType_Map ) ) )
        return;
*/

    /*
    CampaignObjectBase* parentObject;
    if(movedObject)
        parentObject = dynamic_cast<CampaignObjectBase*>(movedObject->parent());
    qDebug() << "[CampaignTree] Item Dropped: " << (movedObject ? movedObject->getName() : QString("null")) << ", current parent: " << (parentObject ? parentObject->getName() : QString("null"));
    QStandardItem* parentItem = item->parent();
    qDebug() << "[CampaignTree] Item Dropped on: " << item->text() << ", " << item->row() << ", parent: " << (parentItem ? parentItem->text() : QString("null"));
    */

//    QTreeView::dropEvent(event);

    /*
    if(movedObject)
        parentObject = dynamic_cast<CampaignObjectBase*>(movedObject->parent());
    qDebug() << "[CampaignTree] Item Dropped: " << (movedObject ? movedObject->getName() : QString("null")) << ", current parent: " << (parentObject ? parentObject->getName() : QString("null"));
    parentItem = item->parent();
    qDebug() << "[CampaignTree] Item Dropped on: " << item->text() << ", " << item->row() << ", parent: " << (parentItem ? parentItem->text() : QString("null"));
    */
}

void CampaignTree::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);

    /*
    if(!model())
        return;

    // Set the expanded state of the item's children appropriately
    //iterateItemExpanded(childItem);

    // Select the moved item
    setCurrentIndex(model()->index(start, 0, parent));


    updateExpandedState();
    */

    /*
    // Update the expand state of the tree items
    //updateExpandedState();

    if(!parent.isValid())
        return;

    CampaignTreeModel* campaignModel = dynamic_cast<CampaignTreeModel*>(model());
    if(!campaignModel)
        return;

    QStandardItem* parentItem = campaignModel->itemFromIndex(parent);
    if(!parentItem)
        return;

    CampaignObjectBase* parentObject = reinterpret_cast<CampaignObjectBase*>(parentItem->data(DMHelper::TreeItemData_Object).value<quint64>());
    parentObject->setExpanded(true);
    setExpanded(parent, true);
    */
    /*
    iterateItemExpanded(parentItem);

    CampaignObjectBase* parentObject = reinterpret_cast<CampaignObjectBase*>(parentItem->data(DMHelper::TreeItemData_Object).value<quint64>());
    QStandardItem* movedItem = parentItem->child(start);
    if(movedItem)
    {
        CampaignObjectBase* movedObject = reinterpret_cast<CampaignObjectBase*>(movedItem->data(DMHelper::TreeItemData_Object).value<quint64>());
        if((movedObject)&&(parentObject))
        {
            qDebug() << "[CampaignTree] Row Inserted" << movedObject->getName() << "(" << ((void*)movedObject) << "), ID " << movedObject->getID() << " to parent object " << parentObject->getName() << "(" << ((void*)parentObject) << "), ID " << parentObject->getID();
        }
    }
    */

    /*
    QModelIndex newEntry = campaignModel->index(start, 0, parent);
    if(newEntry.isValid())
    {
        QStandardItem* movedItem = campaignModel->itemFromIndex(newEntry);
        if(movedItem)
        {
            CampaignObjectBase* movedObject = reinterpret_cast<CampaignObjectBase*>(movedItem->data(DMHelper::TreeItemData_Object).value<quint64>());
            if((movedObject)&&(parentObject))
            {
                qDebug() << "[CampaignTree] Row Inserted" << movedObject->getName() << "(" << ((void*)movedObject) << "), ID " << movedObject->getID() << " to parent object " << parentObject->getName() << "(" << ((void*)parentObject) << "), ID " << parentObject->getID();
            }
        }
    }
    */
}

void CampaignTree::updateExpandedState()
{
    CampaignTreeModel* campaignModel = dynamic_cast<CampaignTreeModel*>(model());
    if(!campaignModel)
        return;

    QStandardItem* rootItem = campaignModel->invisibleRootItem();
    if(!rootItem)
        return;

    for(int i = 0; i < rootItem->rowCount(); ++i)
    {
        iterateItemExpanded(rootItem->child(i));
    }
}

void CampaignTree::iterateItemExpanded(QStandardItem* item)
{
    if(!item)
        return;

//    CampaignObjectBase* itemObject = static_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<void*>());
    CampaignObjectBase* itemObject = reinterpret_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<quint64>());
    if(!itemObject)
        return;

    setExpanded(item->index(), itemObject->getExpanded());

    if(itemObject->getExpanded())
    {
        for(int i = 0; i < item->rowCount(); ++i)
        {
            iterateItemExpanded(item->child(i));
        }
    }
}
