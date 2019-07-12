#ifndef CAMPAIGNTREE_H
#define CAMPAIGNTREE_H

#include <QTreeView>

class CampaignTree : public QTreeView
{
    Q_OBJECT
public:
    explicit CampaignTree(QWidget *parent = nullptr);

signals:

public slots:

protected:
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dropEvent(QDropEvent * event);
};

#endif // CAMPAIGNTREE_H
