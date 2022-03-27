#ifndef OBJECTIMPORTDIALOG_H
#define OBJECTIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ObjectImportDialog;
}

class Campaign;
class CampaignObjectBase;
class ObjectImportWorker;
class DMHWaitingDialog;

class ObjectImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectImportDialog(Campaign* campaign, CampaignObjectBase* parentObject, const QString& campaignFile, QWidget *parent = nullptr);
    ~ObjectImportDialog();

signals:
    void importComplete(bool success);

private slots:
    void selectImportFile();
    void selectAssetDirectory();
    void targetObjectSelected();
    void checkImportValid();

    void runImport();
    void importFinished(bool success, const QString& error);

private:
    bool isImportValid();
    void populateCampaign();
    void addCampaignObject(CampaignObjectBase* object, int depth, const QUuid& selectedObject);
    QIcon objectIcon(CampaignObjectBase* object);

    Ui::ObjectImportDialog *ui;

    Campaign* _campaign;
    CampaignObjectBase* _parentObject;
    CampaignObjectBase* _targetObject;

    ObjectImportWorker* _worker;
    DMHWaitingDialog* _waitingDlg;
};

#endif // OBJECTIMPORTDIALOG_H
