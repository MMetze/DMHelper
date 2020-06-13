#include "campaigntreeitem.h"
#include "campaignobjectbase.h"
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
    QVariant v = data(DMHelper::TreeItemData_ID);
    QString s = v.toString();
    QUuid u(s);
    return u;
    //return QUuid(data(DMHelper::TreeItemData_ID).toString());
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

int CampaignTreeItem::getCampaignItemRow() const
{
    CampaignObjectBase* campaignObject = getCampaignItemObject();
    return (campaignObject == nullptr) ? -1 : campaignObject->getRow();
}

void CampaignTreeItem::setCampaignItemRow(int itemRow)
{
    CampaignObjectBase* campaignObject = getCampaignItemObject();
    if(campaignObject)
        campaignObject->setRow(itemRow);
}

CampaignTreeItem* CampaignTreeItem::getChildById(const QUuid& itemId) const
{
    for(int i = 0; i < rowCount(); ++i)
    {
        CampaignTreeItem* item = dynamic_cast<CampaignTreeItem*>(child(i));
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

CampaignTreeItem* CampaignTreeItem::getChildCampaignItem(int childRow) const
{
    return dynamic_cast<CampaignTreeItem*>(child(childRow));
}
