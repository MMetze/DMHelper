#ifndef MAPMANAGERDIALOG_H
#define MAPMANAGERDIALOG_H

#include <QDialog>

namespace Ui {
class MapManagerDialog;
}

class OptionsContainer;
class QStandardItemModel;
class QSortFilterProxyModel;
class QStandardItem;
class QItemSelection;

class MapManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapManagerDialog(OptionsContainer& options, QWidget *parent = nullptr);
    ~MapManagerDialog();

protected slots:
    void selectItem(const QItemSelection &current, const QItemSelection &previous);
    void openPreviewDialog(const QModelIndex &current);

    void browsePath();
    void findMaps();
    void scanNextDirectory();
    void scanDirectory(QStandardItem* parent, const QString& absolutePath);

private:
    void readModel();
    void writeModel();

    Ui::MapManagerDialog *ui;

    QStandardItemModel* _model;
    QSortFilterProxyModel* _proxy;
    OptionsContainer& _options;
    QList<QPair<QStandardItem*, QString>> _searchList;
};

#endif // MAPMANAGERDIALOG_H
