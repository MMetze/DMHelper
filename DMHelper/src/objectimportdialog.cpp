#include "objectimportdialog.h"
#include "ui_objectimportdialog.h"
#include "dmconstants.h"
#include "campaign.h"
#include "character.h"
#include "audiotrack.h"
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
    _targetObject(nullptr),
    _worker(nullptr),
    _waitingDlg(nullptr)
{
    ui->setupUi(this);
    ui->btnImport->setEnabled(false);

    if(!campaignFile.isEmpty())
        ui->edtAssetDirectory->setText(QFileInfo(campaignFile).absolutePath());

    connect(ui->edtImportFile, &QLineEdit::textChanged, this, &ObjectImportDialog::checkImportValid);
    connect(ui->edtAssetDirectory, &QLineEdit::textChanged, this, &ObjectImportDialog::checkImportValid);
    connect(ui->cmbLocation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ObjectImportDialog::targetObjectSelected);
    connect(ui->btnImportFile, &QAbstractButton::clicked, this, &ObjectImportDialog::selectImportFile);
    connect(ui->btnAssetDirectory, &QAbstractButton::clicked, this, &ObjectImportDialog::selectAssetDirectory);
    connect(ui->btnImport, &QAbstractButton::clicked, this, &ObjectImportDialog::runImport);

    populateCampaign();
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

void ObjectImportDialog::targetObjectSelected()
{
    if(!_campaign)
        return;

    if(ui->cmbLocation->currentIndex() == -1)
        _targetObject = nullptr;
    else
    {
        QUuid targetId(ui->cmbLocation->currentData().toString());
        if(targetId == _campaign->getID())
            _targetObject = _campaign;
        else
            _targetObject = _campaign->searchChildrenById(targetId);
    }

    checkImportValid();
}

void ObjectImportDialog::checkImportValid()
{
    ui->btnImport->setEnabled(isImportValid());
}

void ObjectImportDialog::runImport()
{
    _worker = new ObjectImportWorker();
    if(!_worker->setImportObject(_campaign, _targetObject, ui->edtImportFile->text(), ui->edtAssetDirectory->text(), ui->chkOverwriteAssets->isChecked()))
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

    if((_campaign) && (success))
    {
        if(!_campaign->correctDuplicateIds())
        {
            QMessageBox::information(this,
                                     QString("Duplicate Entries for Loki"),
                                     QString("After the import, some duplicate entries in your campaign have been created. This may have happened to avoid accidentally losing information."));
        }
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

bool ObjectImportDialog::isImportValid()
{
    if(!_campaign)
        return false;

    if((ui->edtImportFile->text().isEmpty()) || (!QFile::exists(ui->edtImportFile->text())))
        return false;

    if((ui->edtAssetDirectory->text().isEmpty()) || (!QDir(ui->edtAssetDirectory->text()).exists()))
        return false;

    if((ui->cmbLocation->currentIndex() == -1) || (!_targetObject))
        return false;

    return true;
}

void ObjectImportDialog::populateCampaign()
{
//    if(!_campaign)
//        return;

    addCampaignObject(_campaign, 0, _parentObject ? _parentObject->getID() : QUuid());
    if((ui->cmbLocation->currentIndex() == -1) && (ui->cmbLocation->count() > 0))
        ui->cmbLocation->setCurrentIndex(0);

//    QList<CampaignObjectBase*> childList = _campaign->getChildObjects();
//    for(CampaignObjectBase* childObject : childList)
//    {
//        addCampaignObject(childObject, 0, _parentObject->getID());
//    }
}

void ObjectImportDialog::addCampaignObject(CampaignObjectBase* object, int depth, const QUuid& selectedObject)
{
    if(!object)
        return;

    QString itemName;
    if(depth > 3)
        itemName.fill(' ', depth - 3);
    itemName.prepend("   ");
    if(depth > 0)
        itemName.append(QString("|--"));
    itemName.append(object->getName());
    ui->cmbLocation->addItem(objectIcon(object), itemName, QVariant(object->getID().toString()));
    if(object->getID() == selectedObject)
        ui->cmbLocation->setCurrentIndex(ui->cmbLocation->count() - 1);

    QList<CampaignObjectBase*> childList = object->getChildObjects();
    for(CampaignObjectBase* childObject : childList)
    {
        addCampaignObject(childObject, depth + 3, selectedObject);
    }
}

QIcon ObjectImportDialog::objectIcon(CampaignObjectBase* object)
{
    if(!object)
        return QIcon();

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            return QIcon(":/img/data/icon_contentparty.png");
        case DMHelper::CampaignType_Combatant:
            {
                Character* character = dynamic_cast<Character*>(object);
                return ((character) && (character->isInParty())) ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png");
            }
        case DMHelper::CampaignType_Map:
            return QIcon(":/img/data/icon_contentmap.png");
        case DMHelper::CampaignType_Text:
        case DMHelper::CampaignType_LinkedText:
            return QIcon(":/img/data/icon_contenttextencounter.png");
        case DMHelper::CampaignType_Battle:
            return QIcon(":/img/data/icon_contentbattle.png");
        case DMHelper::CampaignType_ScrollingText:
            return QIcon(":/img/data/icon_contentscrollingtext.png");
        case DMHelper::CampaignType_AudioTrack:
            {
                AudioTrack* track = dynamic_cast<AudioTrack*>(object);
                if((track) && (track->getAudioType() == DMHelper::AudioType_Syrinscape))
                    return QIcon(":/img/data/icon_syrinscape.png");
                else if((track) && (track->getAudioType() == DMHelper::AudioType_Youtube))
                    return QIcon(":/img/data/icon_playerswindow.png");
                else
                    return QIcon(":/img/data/icon_soundboard.png");
            }
        default:
            return QIcon();
    }
}

