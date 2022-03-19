#include "objectimporter.h"
#include "objectimportworker.h"
#include "dmhwaitingdialog.h"
#include "campaignobjectbase.h"
//#include <QThread>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

//const int IMPORT_THREAD_DONE = 0x01;
//const int IMPORT_COMPLETED = 0x02;
//const int IMPORT_ALL_DONE = IMPORT_THREAD_DONE | IMPORT_COMPLETED;

ObjectImporter::ObjectImporter(QObject *parent) :
    QObject(parent),
    //_workerThread(nullptr),
    _worker(nullptr),
    _waitingDlg(nullptr)//,
 //   _completeValue(0)
{
}

ObjectImporter::~ObjectImporter()
{
}

bool ObjectImporter::importObject(Campaign* campaign, CampaignObjectBase* parentObject, const QString& campaignFile)
{
    QString importFilename = QFileDialog::getOpenFileName(nullptr,QString("Select file to import"), QString(), QString("XML files (*.xml)"));
    if((importFilename.isEmpty()) || (!QFile::exists(importFilename)))
    {
        qDebug() << "[ObjectImporter] Not able to find the selected import file: " << importFilename;
        return false;
    }

    QFileInfo currentFileInfo (campaignFile);
    QString assetPath = QFileDialog::getExistingDirectory(nullptr,
                                                          QString("Select the location where imported assets should be stored"),
                                                          currentFileInfo.absolutePath());
    if((assetPath.isEmpty()) || (!QDir(assetPath).exists()))
    {
        qDebug() << "[ObjectImporter] Not able to find the selected asset import location: " << assetPath;
        return false;
    }

    bool replaceDuplicates = QMessageBox::critical(nullptr,
                                                   QString("Duplicate import IDs"),
                                                   QString("If the imported file contains items that are already in the current campaign, do you want to overwrite the existing items?"),
                                                   QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

    qDebug() << "[ObjectImporter] Importing file " << importFilename << " to campaign file " << campaignFile << " into parent object " << parentObject->getName();

    //_workerThread = new QThread(this);
    _worker = new ObjectImportWorker();
    if(!_worker->setImportObject(campaign, parentObject, importFilename, assetPath, replaceDuplicates))
    {
        qDebug() << "[ObjectImporter] Not able to set import object information!";
        delete _worker; _worker = nullptr;
        //delete _workerThread; _workerThread = nullptr;
        return false;
    }

    //_worker->moveToThread(_workerThread);
    //connect(_workerThread, &QThread::finished, this, &ObjectImporter::threadFinished);
    connect(_worker, &ObjectImportWorker::workComplete, this, &ObjectImporter::importFinished);
    //connect(this, &ObjectImporter::startWork, _worker, &ObjectImportWorker::doWork);

    if(!_waitingDlg)
        _waitingDlg = new DMHWaitingDialog(QString("Importing ") + importFilename + QString("..."));

    connect(_worker, &ObjectImportWorker::updateStatus, _waitingDlg, &DMHWaitingDialog::setSplitStatus);
    //_workerThread->start();
    //emit startWork();
    _waitingDlg->setModal(true);
    _waitingDlg->show();

    _worker->doWork();

    return true;
}

void ObjectImporter::importFinished(bool success, const QString& error)
{
    qDebug() << "[ObjectImporter] Import completed, stopping dialog and thread. Success: " << success << ", error string: " << error;

    if(_waitingDlg)
    {
        _waitingDlg->accept();
        _waitingDlg->deleteLater();
        _waitingDlg = nullptr;
    }

    //if(_workerThread)
    //    _workerThread->quit();

    if(success)
        QMessageBox::information(nullptr, QString("DMHelper - Import Object"), QString("Import completed successfully!"));
    else
        QMessageBox::critical(nullptr, QString("DMHelper - Import Object"), QString("Import could not be completed: ") + error);

    qDebug() << "[ObjectImporter] Import worker finished.";
    emit importComplete(success);

    if(_worker)
    {
        _worker->deleteLater();
        _worker = nullptr;
    }

    deleteLater();
}
