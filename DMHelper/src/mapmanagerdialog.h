#ifndef MAPMANAGERDIALOG_H
#define MAPMANAGERDIALOG_H

#include <QDialog>

namespace Ui {
class MapManagerDialog;
}

class OptionsContainer;
class QTreeWidgetItem;

class MapManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapManagerDialog(OptionsContainer& options, QWidget *parent = nullptr);
    ~MapManagerDialog();

protected slots:
    void selectItem(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void openPreviewDialog(QTreeWidgetItem *item, int column);

    void browsePath();
    void findMaps();
    void scanNextDirectory();
    void scanDirectory(QTreeWidgetItem* parent, const QString& absolutePath);

private:
    Ui::MapManagerDialog *ui;

    OptionsContainer& _options;
    QList<QPair<QTreeWidgetItem*, QString>> _searchList;
};

#endif // MAPMANAGERDIALOG_H
