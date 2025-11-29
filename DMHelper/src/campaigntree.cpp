#include "campaigntree.h"
#include "dmconstants.h"
#include "campaigntreemodel.h"
#include "campaignobjectbase.h"
#include <QStandardItemModel>
#include <QDropEvent>
#include <QMimeDatabase>
#include <QMimeData>
#include <QDebug>

CampaignTree::CampaignTree(QWidget *parent) :
    QTreeView(parent)
{
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
}

CampaignObjectBase* CampaignTree::currentCampaignObject()
{
    CampaignTreeModel* campaignModel = dynamic_cast<CampaignTreeModel*>(model());
    if(!campaignModel)
        return nullptr;

    QStandardItem* item = campaignModel->itemFromIndex(currentIndex());
    if(!item)
        return nullptr;

    return reinterpret_cast<CampaignObjectBase*>(item->data(DMHelper::TreeItemData_Object).value<quint64>());
}

void CampaignTree::campaignChanged()
{
    updateExpandedState();
}

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
}

void CampaignTree::publishCurrent(bool publish)
{
    CampaignTreeModel* campaignModel = dynamic_cast<CampaignTreeModel*>(model());
    if(campaignModel)
        campaignModel->setPublishIndex(currentIndex(), publish);
}

void CampaignTree::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* data = event->mimeData();

    if((data->hasUrls()) &&
        (data->urls().count() == 1) &&
        (data->urls().constFirst().isLocalFile()))
    {
        QString filename = data->urls().constFirst().toLocalFile();
        QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filename);
        if((mimeType.isValid()) &&
            ((mimeType.name().startsWith("image/")) ||
             (mimeType.name().startsWith("video/")) ||
             (mimeType.name().startsWith("text/")) ||
             (mimeType.suffixes().contains("xml"))))
        {
            event->acceptProposedAction();
            return;
        }
    }

    QTreeView::dragEnterEvent(event);
}

void CampaignTree::dragMoveEvent(QDragMoveEvent * event)
{
    const QMimeData* data = event->mimeData();
    if((data->hasUrls()) &&
        (data->urls().count() == 1) &&
        (data->urls().constFirst().isLocalFile()))
    {
        event->acceptProposedAction();
        return;
    }

    QTreeView::dragMoveEvent(event);

    if (!event->isAccepted())
        return;

    event->accept();
}

void CampaignTree::dropEvent(QDropEvent *event)
{
    QModelIndex index = indexAt(event->position().toPoint());
    const QMimeData* data = event->mimeData();

    if((data->hasUrls()) &&
        (data->urls().count() == 1) &&
        (data->urls().constFirst().isLocalFile()))
    {
        QString filename = data->urls().constFirst().toLocalFile();
        QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filename);
        if((mimeType.isValid()) &&
            ((mimeType.name().startsWith("image/")) ||
             (mimeType.name().startsWith("video/")) ||
             (mimeType.name().startsWith("text/")) ||
             (mimeType.suffixes().contains("xml"))))
        {
            emit treeDrop(index, filename);
            event->acceptProposedAction();
            return;
        }
    }

    QTreeView::dropEvent(event);
}

void CampaignTree::keyPressEvent(QKeyEvent *event)
{
    if(event)
    {
        if(event->modifiers() == Qt::AltModifier)
        {
            event->ignore();
            return;
        }
        else if(event->key() == Qt::Key_F2)
        {
            QModelIndex index = currentIndex();
            if(index.isValid())
                edit(index);
            return;
        }
    }

    QTreeView::keyPressEvent(event);
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
