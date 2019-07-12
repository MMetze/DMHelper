#ifndef CAMPAIGNTREEMODEL_H
#define CAMPAIGNTREEMODEL_H

#include <QStandardItemModel>

class CampaignTreeModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit CampaignTreeModel(QObject *parent = nullptr);

signals:

public slots:

    // From QAbstractItemModel
    virtual QMimeData *	mimeData(const QModelIndexList & indexes) const;
    virtual QStringList	mimeTypes() const;

};

#endif // CAMPAIGNTREEMODEL_H
