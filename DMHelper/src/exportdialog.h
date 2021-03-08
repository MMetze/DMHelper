#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class QUuid;
class Campaign;
class QTreeWidgetItem;
class CampaignObjectBase;

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(const Campaign& campaign, const QUuid& selectedItem, QWidget *parent = nullptr);
    virtual ~ExportDialog() override;

    QTreeWidgetItem* createChildObject(const CampaignObjectBase* childObject, const QUuid& selectedItem);

private slots:
    void handleItemChanged(QTreeWidgetItem *item, int column);

private:
    void setRecursiveChecked(QTreeWidgetItem *item, bool checked);
    void setObjectIcon(const CampaignObjectBase* baseObject, QTreeWidgetItem* widgetItem);

    Ui::ExportDialog *ui;

    const Campaign& _campaign;
    const QUuid& _selectedItem;

};

#endif // EXPORTDIALOG_H
