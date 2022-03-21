#include "objectimportdialog.h"
#include "ui_objectimportdialog.h"
#include "campaign.h"
#include "objectimportworker.h"
#include "dmhwaitingdialog.h"
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QDebug>

ObjectImportDialog::ObjectImportDialog(Campaign* campaign, CampaignObjectBase* parentObject, const QString& campaignFile, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ObjectImportDialog),
    _campaign(campaign),
    _parentObject(parentObject),
    _worker(nullptr),
    _waitingDlg(nullptr)
{
    ui->setupUi(this);
    ui->btnImport->setEnabled(false);

    if(!campaignFile.isEmpty())
        ui->edtAssetDirectory->setText(QFileInfo(campaignFile).absolutePath());

    connect(ui->edtImportFile, &QLineEdit::textChanged, this, &ObjectImportDialog::importFileChanged);
    connect(ui->edtAssetDirectory, &QLineEdit::textChanged, this, &ObjectImportDialog::assetDirectoryChanged);
    connect(ui->btnImportFile, &QAbstractButton::clicked, this, &ObjectImportDialog::selectImportFile);
    connect(ui->btnAssetDirectory, &QAbstractButton::clicked, this, &ObjectImportDialog::selectAssetDirectory);
    connect(ui->btnImport, &QAbstractButton::clicked, this, &ObjectImportDialog::runImport);
}

ObjectImportDialog::~ObjectImportDialog()
{
    delete ui;
}

void ObjectImportDialog::selectImportFile()
{
    QString importFilename = QFileDialog::getOpenFileName(nullptr,QString("Select file to import"), QString(), QString("XML files (*.xml)"));
    if(!importFilename.isEmpty())
        ui->edtImportFile->setText(importFilename);
}

void ObjectImportDialog::selectAssetDirectory()
{
    QString assetPath = QFileDialog::getExistingDirectory(nullptr,
                                                          QString("Select the location where imported assets should be stored"),
                                                          QFileInfo(ui->edtAssetDirectory->text()).absolutePath());
    if(!assetPath.isEmpty())
        ui->edtAssetDirectory->setText(assetPath);
}

void ObjectImportDialog::importFileChanged()
{
    checkImportValid();
}

void ObjectImportDialog::assetDirectoryChanged()
{
    checkImportValid();
}

void ObjectImportDialog::runImport()
{
    _worker = new ObjectImportWorker();
    if(!_worker->setImportObject(_campaign, _parentObject, ui->edtImportFile->text(), ui->edtAssetDirectory->text(), ui->chkOverwriteAssets->isChecked()))
    {
        qDebug() << "[ObjectImportDialog] Not able to set import object information!";
        delete _worker; _worker = nullptr;
        return;
    }

    connect(_worker, &ObjectImportWorker::workComplete, this, &ObjectImportDialog::importFinished);

    if(!_waitingDlg)
    {
        _waitingDlg = new DMHWaitingDialog(QString("Importing ") + ui->edtImportFile->text() + QString("..."));
        _waitingDlg->resize(width() * 2 / 3, _waitingDlg->height() * 3 / 2);
    }

    connect(_worker, &ObjectImportWorker::updateStatus, _waitingDlg, &DMHWaitingDialog::setSplitStatus);
    _waitingDlg->setModal(true);
    _waitingDlg->show();

    _worker->doWork();
}

void ObjectImportDialog::importFinished(bool success, const QString& error)
{
    qDebug() << "[ObjectImportDialog] Import completed, stopping waiting dialog. Success: " << success << ", error string: " << error;

    if(_waitingDlg)
    {
        _waitingDlg->accept();
        _waitingDlg->deleteLater();
        _waitingDlg = nullptr;
    }

    if(success)
        QMessageBox::information(nullptr, QString("DMHelper - Import Object"), QString("Import completed successfully!"));
    else
        QMessageBox::critical(nullptr, QString("DMHelper - Import Object"), QString("Import could not be completed: ") + error);

    qDebug() << "[ObjectImportDialog] Import worker finished.";
    emit importComplete(success);

    if(_worker)
    {
        _worker->deleteLater();
        _worker = nullptr;
    }

    accept();
}

void ObjectImportDialog::checkImportValid()
{
    ui->btnImport->setEnabled(isImportValid());
}

bool ObjectImportDialog::isImportValid()
{
    if((ui->edtImportFile->text().isEmpty()) || (!QFile::exists(ui->edtImportFile->text())))
        return false;

    if((ui->edtAssetDirectory->text().isEmpty()) || (!QDir(ui->edtAssetDirectory->text()).exists()))
        return false;

    return true;
}
