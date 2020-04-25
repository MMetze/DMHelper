#include "campaigntree.h"
#include "dmconstants.h"
#include "campaigntreemodel.h"
#include "campaignobjectbase.h"
#include <QDropEvent>
#include <QStandardItemModel>
#include <QMimeData>

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
    return reinterpret_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<uintptr_t>());
}

void CampaignTree::campaignChanged()
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
    event->ignore();

    const QMimeData* data = event->mimeData();
    QByteArray encodedData = data->data(QString("application/vnd.dmhelper.text"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int treeItemDataType;
    int treeItemDataID;
    stream >> treeItemDataType >> treeItemDataID;

    QModelIndex index = indexAt(event->pos());
    if(!index.isValid())
        return;

    QStandardItemModel* stdModel = dynamic_cast<QStandardItemModel*>(model());
    if(!stdModel)
        return;

    QStandardItem* item = stdModel->itemFromIndex(index);
    if(!item)
        return;

    /*
    int itemType = item->data(DMHelper::TreeItemData_Type).toInt();
    if( (itemType != treeItemDataType) ||
        ( ( itemType != DMHelper::TreeType_Encounter ) &&
          ( itemType != DMHelper::TreeType_Map ) ) )
        return;
*/
    QTreeView::dropEvent(event);
}

void CampaignTree::iterateItemExpanded(QStandardItem* item)
{
    if(!item)
        return;

//    CampaignObjectBase* itemObject = static_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<void*>());
    CampaignObjectBase* itemObject = reinterpret_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<uintptr_t>());
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
