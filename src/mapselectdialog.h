#ifndef MAPSELECTDIALOG_H
#define MAPSELECTDIALOG_H

#include <QDialog>

namespace Ui {
class MapSelectDialog;
}

class Map;
class QListWidgetItem;

class MapSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapSelectDialog(QWidget *parent = 0);
    ~MapSelectDialog();

    void addMap(Map* newMap);
    //void addSeparator(int index);

    Map* getSelectedMap() const;

private slots:
    void handleItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::MapSelectDialog *ui;
};

#endif // MAPSELECTDIALOG_H
