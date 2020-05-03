#ifndef CAMPAIGNTREEMODEL_H
#define CAMPAIGNTREEMODEL_H

#include "campaign.h"
#include <QStandardItemModel>

class Campaign;
class CampaignObjectBase;
class CampaignTreeItem;

class CampaignTreeModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit CampaignTreeModel(QObject *parent = nullptr);

    Campaign* getCampaign() const;

    bool containsObject(const QUuid& objectId) const;
    QModelIndex getObject(const QUuid& objectId) const;

    CampaignTreeItem *campaignItem(int row, int column = 0) const;
    CampaignTreeItem *campaignItemFromIndex(const QModelIndex &index) const;

signals:
    void campaignChanged(Campaign* campaign);
    void itemMoved(QStandardItem* parentItem, int row);

public slots:

    // From QAbstractItemModel
    virtual QMimeData *	mimeData(const QModelIndexList & indexes) const override;
    virtual QStringList	mimeTypes() const override;

    void setCampaign(Campaign* campaign);
    void refresh();

protected slots:
    void handleRowsInserted(const QModelIndex &parent, int first, int last);
//    void handleRowsRemoved(const QModelIndex &parent, int first, int last);
    void handleTimer();

protected:

    void updateCampaignEntries();
    QStandardItem* createTreeEntry(CampaignObjectBase* object, QStandardItem* parentEntry);
    void appendTreeEntry(QStandardItem* objectEntry, QStandardItem* parentEntry);
    void validateChildStructure(QStandardItem* parentItem);
    void validateIndividualChild(QStandardItem* parentItem, int row);
    void resetItemExpansion(QStandardItem* item);

    Campaign* _campaign;
    QMap<QUuid, QModelIndex> _objectIndexMap;
    QStandardItem* _updateParent;
    int _updateRow;
};

#endif // CAMPAIGNTREEMODEL_H
