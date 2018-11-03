#include "campaigntree.h"
#include "dmconstants.h"
#include <QDropEvent>
#include <QStandardItemModel>
#include <QMimeData>

CampaignTree::CampaignTree(QWidget *parent) :
    QTreeView(parent)
{
}

void CampaignTree::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeView::dragMoveEvent(event);

    if (!event->isAccepted())
        return;

    if((dropIndicatorPosition() == AboveItem) || (dropIndicatorPosition() == BelowItem))
        event->accept();
    else
        event->ignore(); //Show 'forbidden' cursor.
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

    int itemType = item->data(DMHelper::TreeItemData_Type).toInt();
    if( (itemType != treeItemDataType) ||
        ( ( itemType != DMHelper::TreeType_Encounter ) &&
          ( itemType != DMHelper::TreeType_Map ) ) )
        return;

    QTreeView::dropEvent(event);
}
