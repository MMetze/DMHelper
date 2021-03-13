#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class Campaign;
class CampaignObjectBase;
class QUuid;
class QTreeWidgetItem;
class ExportWorker;
class DMHWaitingDialog;
class QThread;

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

signals:
    void startWork();

private slots:
    void handleCampaignItemChanged(QTreeWidgetItem *item, int column);
    void runExport();
    void exportFinished(bool success);
    void threadFinished();

private:
    void setRecursiveChecked(QTreeWidgetItem *item, bool checked);
    void setObjectIcon(const CampaignObjectBase* baseObject, QTreeWidgetItem* widgetItem);
    void checkObjectContent(const CampaignObjectBase* object);
    void refreshMonsters();
    void recursiveRefreshMonsters(QTreeWidgetItem* widgetItem);

    Ui::ExportDialog *ui;

    const Campaign& _campaign;
    const QUuid& _selectedItem;
    QStringList _monsters;

    QThread* _workerThread;
    ExportWorker* _worker;
    DMHWaitingDialog* _waitingDlg;

};

#endif // EXPORTDIALOG_H
