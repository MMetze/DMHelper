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
    void treeDrop(const QModelIndex & index, const QString& filename);

public slots:
    void campaignChanged();
    void handleItemMoved(QStandardItem* movedItem);
    void publishCurrent(bool publish);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent * event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

    void updateExpandedState();
    void iterateItemExpanded(QStandardItem* item);
};

#endif // CAMPAIGNTREE_H
