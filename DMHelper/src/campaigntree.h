#ifndef CAMPAIGNTREE_H
#define CAMPAIGNTREE_H

#include <QTreeView>

class QStandardItem;
class CampaignObjectBase;

class CampaignTree : public QTreeView
{
    Q_OBJECT
public:
    explicit CampaignTree(QWidget *parent = nullptr);

    CampaignObjectBase* currentCampaignObject();

signals:

public slots:
    void campaignChanged();
    void handleItemMoved(QStandardItem* parentItem, int row);

protected:
    virtual void dragMoveEvent(QDragMoveEvent * event) override;
    virtual void dropEvent(QDropEvent * event) override;
    virtual void rowsInserted(const QModelIndex &parent, int start, int end) override;

    void updateExpandedState();
    void iterateItemExpanded(QStandardItem* item);
};

#endif // CAMPAIGNTREE_H
