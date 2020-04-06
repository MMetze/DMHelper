#include "campaigntreemodel.h"
#include "dmconstants.h"
#include <QMimeData>
#include <QUuid>

CampaignTreeModel::CampaignTreeModel(QObject *parent) :
    QStandardItemModel(parent),
    _campaign(nullptr)
{
}

Campaign* CampaignTreeModel::getCampaign() const
{
    return _campaign;
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

void CampaignTreeModel::updateCampaignEntries()
{
    if(!_campaign)
        return;

    clear();

    QList<CampaignObjectBase*> childObjects = _campaign->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        QStandardItem* objectEntry = createTreeEntry(childObject);
        if(objectEntry)
            appendRow(objectEntry);
    }
}

QStandardItem* CampaignTreeModel::createTreeEntry(CampaignObjectBase* object)
{
    if(!object)
        return nullptr;

    QStandardItem* treeEntry = new QStandardItem(object->getName());
    treeEntry->setEditable(true);

    treeEntry->setData(QVariant::fromValue(static_cast<void*>(object)),DMHelper::TreeItemData_Object);
    //campaignItem->setData(QVariant(DMHelper::TreeType_Campaign),DMHelper::TreeItemData_Type);
    //campaignItem->setData(QVariant(QString()),DMHelper::TreeItemData_ID);
    //treeModel->appendRow(campaignItem);
    //ui->treeView->expand(campaignItem->index());

    QList<CampaignObjectBase*> childObjects = object->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        QStandardItem* objectEntry = createTreeEntry(childObject);
        if(objectEntry)
            treeEntry->appendRow(objectEntry);
    }

    return treeEntry;
}


