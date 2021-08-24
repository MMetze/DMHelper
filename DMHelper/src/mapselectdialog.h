#ifndef MAPSELECTDIALOG_H
#define MAPSELECTDIALOG_H

#include <QDialog>
#include <QUuid>

namespace Ui {
class MapSelectDialog;
}

class Campaign;
class CampaignObjectBase;
class Map;
class QTreeWidgetItem;

class MapSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapSelectDialog(Campaign& campaign, const QUuid& currentId, QWidget *parent = nullptr);
    ~MapSelectDialog();

    //void prependMap(Map* newMap);
    //void appendMap(Map* newMap);
    //void addSeparator(int index);

    Map* getSelectedMap() const;

protected slots:
    void handleItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void setupSelectTree(Campaign& campaign, const QUuid& currentId);
    bool insertObject(CampaignObjectBase* object, QTreeWidgetItem* parentItem, const QUuid& currentId);
    void decorateItem(QTreeWidgetItem* item, CampaignObjectBase* object);

protected:
    Ui::MapSelectDialog *ui;

    QTreeWidgetItem* _currentItem;
};

#endif // MAPSELECTDIALOG_H
