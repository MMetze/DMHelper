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
    CampaignTreeItem(const CampaignTreeItem& other);
    virtual ~CampaignTreeItem() override;

    virtual QStandardItem* clone() const override;
    virtual int type() const override;

    virtual int getCampaignItemType() const;
    virtual void setCampaignItemType(int itemType);

    virtual QUuid getCampaignItemId() const;
    virtual void setCampaignItemId(const QUuid& itemId);

    virtual CampaignObjectBase* getCampaignItemObject() const;
    virtual void setCampaignItemObject(CampaignObjectBase* itemObject);

    virtual int getCampaignItemRow() const;
    virtual void setCampaignItemRow(int itemRow);

    virtual CampaignTreeItem* getChildById(const QUuid& itemId) const;
    virtual CampaignTreeItem* getChildCampaignItem(int childRow) const;

    void setPublishing(bool publishing);
    void updateVisualization();

protected:
    void setVisualization();

    bool _isPublishing = false;
};

#endif // CAMPAIGNTREEITEM_H
