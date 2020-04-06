#ifndef CAMPAIGNTREE_H
#define CAMPAIGNTREE_H

#include <QTreeView>

class QStandardItem;

class CampaignTree : public QTreeView
{
    Q_OBJECT
public:
    explicit CampaignTree(QWidget *parent = nullptr);

signals:

public slots:
    void campaignChanged();

protected:
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);

    void iterateItemExpanded(QStandardItem* item);
};

#endif // CAMPAIGNTREE_H
