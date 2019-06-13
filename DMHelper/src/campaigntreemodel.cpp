#include "campaigntreemodel.h"
#include "dmconstants.h"
#include <QMimeData>
#include <QUuid>

CampaignTreeModel::CampaignTreeModel(QObject *parent) :
    QStandardItemModel(parent)
{
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
