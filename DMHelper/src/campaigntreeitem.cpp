#include "campaigntreeitem.h"
#include "dmconstants.h"
#include <QUuid>

CampaignTreeItem::CampaignTreeItem() :
    QStandardItem()
{
}

CampaignTreeItem::CampaignTreeItem(int rows, int columns) :
    QStandardItem(rows, columns)
{
}

CampaignTreeItem::CampaignTreeItem(const QIcon &icon, const QString &text) :
    QStandardItem(icon, text)
{
}

CampaignTreeItem::CampaignTreeItem(const QString &text) :
    QStandardItem(text)
{
}

CampaignTreeItem::~CampaignTreeItem()
{
}

QStandardItem* CampaignTreeItem::clone() const
{
    //return QStandardItem::clone();
    return new CampaignTreeItem();
}

int CampaignTreeItem::type() const
{
    return Qt::UserRole;
}

int CampaignTreeItem::getCampaignItemType() const
{
    return data(DMHelper::TreeItemData_Type).toInt();
}

void CampaignTreeItem::setCampaignItemType(int itemType)
{
    setData(QVariant(itemType), DMHelper::TreeItemData_Type);
}

QUuid CampaignTreeItem::getCampaignItemId() const
{
    return QUuid(data(DMHelper::TreeItemData_ID).toString());
}

void CampaignTreeItem::setCampaignItemId(const QUuid& itemId)
{
    setData(QVariant(itemId.toString()), DMHelper::TreeItemData_ID);
}

CampaignObjectBase* CampaignTreeItem::getCampaignItemObject() const
{
    return reinterpret_cast<CampaignObjectBase*>(data(DMHelper::TreeItemData_Object).value<quint64>());
}

void CampaignTreeItem::setCampaignItemObject(CampaignObjectBase* itemObject)
{
    setData(QVariant::fromValue(reinterpret_cast<quint64>(itemObject)), DMHelper::TreeItemData_Object);
}
