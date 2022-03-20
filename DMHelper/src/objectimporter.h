#ifndef OBJECTIMPORTER_H
#define OBJECTIMPORTER_H

#include <QObject>

class ObjectImportWorker;
class DMHWaitingDialog;
class Campaign;
class CampaignObjectBase;

class ObjectImporter : public QObject
{
    Q_OBJECT
public:
    explicit ObjectImporter(QObject *parent = nullptr);
    virtual ~ObjectImporter() override;

    bool importObject(Campaign* campaign, CampaignObjectBase* parentObject, const QString& campaignFile);

signals:
    //void startWork();
    void importComplete(bool success);
    void statusUpdate(const QString& primary, const QString& secondary);

protected slots:
    void importFinished(bool success, const QString& error);
//    void threadFinished();

protected:
    void completeCheck(int completeValue);

    //QThread* _workerThread;
    ObjectImportWorker* _worker;
    DMHWaitingDialog* _waitingDlg;
    //int _completeValue;
};

#endif // OBJECTIMPORTER_H
