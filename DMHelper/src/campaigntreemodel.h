#ifndef CAMPAIGNTREEMODEL_H
#define CAMPAIGNTREEMODEL_H

#include "campaign.h"
#include <QStandardItemModel>

class Campaign;
class CampaignObjectBase;

class CampaignTreeModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit CampaignTreeModel(QObject *parent = nullptr);

    Campaign* getCampaign() const;

    bool containsObject(const QUuid& objectId) const;
    QModelIndex getObject(const QUuid& objectId) const;

signals:
    void campaignChanged(Campaign* campaign);

public slots:

    // From QAbstractItemModel
    virtual QMimeData *	mimeData(const QModelIndexList & indexes) const;
    virtual QStringList	mimeTypes() const;

    void setCampaign(Campaign* campaign);
    void refresh();

protected:
    void updateCampaignEntries();
    QStandardItem* createTreeEntry(CampaignObjectBase* object, QStandardItem* parentEntry);
    void appendTreeEntry(QStandardItem* objectEntry, QStandardItem* parentEntry);

    Campaign* _campaign;
    QMap<QUuid, QModelIndex> _objectIndexMap;
};

#endif // CAMPAIGNTREEMODEL_H
