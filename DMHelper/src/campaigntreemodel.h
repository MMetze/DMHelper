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

signals:
    void campaignChanged(Campaign* campaign);

public slots:

    // From QAbstractItemModel
    virtual QMimeData *	mimeData(const QModelIndexList & indexes) const;
    virtual QStringList	mimeTypes() const;

    void setCampaign(Campaign* campaign);

protected:
    void updateCampaignEntries();
    QStandardItem* createTreeEntry(CampaignObjectBase* object);

    Campaign* _campaign;
};

#endif // CAMPAIGNTREEMODEL_H
