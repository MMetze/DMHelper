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
class QDomDocument;
class QDomElement;

class MapManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapManagerDialog(OptionsContainer& options, QWidget *parent = nullptr);
    ~MapManagerDialog();

    struct MapFileMetadata
    {
        int _type;
        QString _filePath;
        QStringList _tags;
    };

protected slots:
    virtual void showEvent(QShowEvent *event) override;

    void selectItem(const QItemSelection &current, const QItemSelection &previous);
    void openPreviewDialog(const QModelIndex &current);

    void browsePath();
    void findMaps();
    void scanNextDirectory();
    void scanDirectory(QStandardItem* parent, const QString& absolutePath);

    void readModel();
    void writeModel();

private:
    void setCurrentPath(const QString& path);
    void inputItemXML(QDomElement &element, QStandardItem& parent);
    void outputItemXML(QDomDocument &doc, QDomElement &parent, QStandardItem& item);

    Ui::MapManagerDialog *ui;

    QStandardItemModel* _model;
    QSortFilterProxyModel* _proxy;
    OptionsContainer& _options;
    QString _currentPath;
    QList<QPair<QStandardItem*, QString>> _searchList;
};

Q_DECLARE_METATYPE(MapManagerDialog::MapFileMetadata);

#endif // MAPMANAGERDIALOG_H
