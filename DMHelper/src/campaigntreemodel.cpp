#include "campaigntreemodel.h"
#include "dmconstants.h"
#include <QMimeData>

CampaignTreeModel::CampaignTreeModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

QMimeData *	CampaignTreeModel::mimeData(const QModelIndexList & indexes) const
{
    QMimeData *data = QStandardItemModel::mimeData(indexes);
    if(!data)
        return NULL;

    if(indexes.count() == 1)
    {
        QStandardItem* item = itemFromIndex(indexes.at(0));
        if(item)
        {
            QByteArray encodedData;
            QDataStream stream(&encodedData, QIODevice::WriteOnly);
            stream << item->data(DMHelper::TreeItemData_Type).toInt() << item->data(DMHelper::TreeItemData_ID).toInt();
            data->setData(QString("application/vnd.dmhelper.text"), encodedData);
        }
    }

    return data;
}

QStringList	CampaignTreeModel::mimeTypes() const
{
    return QStandardItemModel::mimeTypes() <<  QLatin1String("application/vnd.dmhelper.text");
}
