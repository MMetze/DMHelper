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
    QModelIndex getObjectIndex(const QUuid& objectId) const;
    CampaignTreeItem* getObjectItemByName(const QString& name) const;
    CampaignTreeItem* getObjectItem(const QUuid& objectId) const;

    CampaignTreeItem* campaignItem(int row, int column = 0) const;
    CampaignTreeItem* campaignItemFromIndex(const QModelIndex &index) const;

    // From QAbstractItemModel
    virtual QMimeData *	mimeData(const QModelIndexList & indexes) const override;
    virtual QStringList	mimeTypes() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

signals:
    void campaignChanged(Campaign* campaign);
    void itemMoved(QStandardItem* movedItem);

public slots:

    void setCampaign(Campaign* campaign);
    void setPublishIndex(const QModelIndex& index, bool publish);
    void refresh();

protected slots:
    void handleRowsInserted(const QModelIndex &parent, int first, int last);
    void handleRowsRemoved(const QModelIndex &parent, int first, int last);
    void handleRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void handleTimer();
    void handleItemChanged(QStandardItem *item);
    void handleObjectNameChanged(CampaignObjectBase* object, const QString& name);

protected:

    void updateCampaignEntries();
    QStandardItem* createTreeEntry(CampaignObjectBase* object, QStandardItem* parentEntry);
    void addTreeEntry(CampaignTreeItem* objectEntry, QStandardItem* parentEntry);
    void validateChildStructure(QStandardItem* parentItem);
    void validateIndividualChild(QStandardItem* parentItem, int row);
    void validateIndividualChild(QStandardItem* parentItem, QStandardItem* childItem);
    void resetItemExpansion(QStandardItem* item);

    void iterateTreeEntryVisualization(CampaignTreeItem* entry);
    void updateChildRows(QStandardItem* parentItem);
    CampaignTreeItem* getChildById(QStandardItem* parentItem, const QUuid& itemId) const;
    QStandardItem* searchChildByName(QStandardItem* parentItem, const QString& name) const;

    Campaign* _campaign;
    QMap<QUuid, QPersistentModelIndex> _objectIndexMap;
    QUuid _publishingItem;
    QStandardItem* _updateParent;
    int _updateRow;
};

#endif // CAMPAIGNTREEMODEL_H
