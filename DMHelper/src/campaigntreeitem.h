#ifndef CAMPAIGNTREEITEM_H
#define CAMPAIGNTREEITEM_H

#include <QStandardItem>

class CampaignObjectBase;

class CampaignTreeItem : public QStandardItem
{
public:

    CampaignTreeItem();
    CampaignTreeItem(int rows, int columns = 1);
    CampaignTreeItem(const QIcon &icon, const QString &text);
    CampaignTreeItem(const QString &text);
    virtual ~CampaignTreeItem() override;

    virtual QStandardItem* clone() const override;
    virtual int type() const override;

    virtual int getCampaignItemType() const;
    virtual void setCampaignItemType(int itemType);

    virtual QUuid getCampaignItemId() const;
    virtual void setCampaignItemId(const QUuid& itemId);

    virtual CampaignObjectBase* getCampaignItemObject() const;
    virtual void setCampaignItemObject(CampaignObjectBase* itemObject);
};

#endif // CAMPAIGNTREEITEM_H
