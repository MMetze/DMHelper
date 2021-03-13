#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "campaign.h"
#include "dmconstants.h"
#include "character.h"
#include "audiotrack.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclass.h"
#include "dmhwaitingdialog.h"
#include "exportworker.h"
#include <QUuid>
#include <QFileDialog>
#include <QThread>
#include <QMessageBox>
#include <QDebug>

ExportDialog::ExportDialog(const Campaign& campaign, const QUuid& selectedItem, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    _campaign(campaign),
    _selectedItem(selectedItem),
    _monsters(),
    _workerThread(nullptr),
    _worker(nullptr),
    _waitingDlg(nullptr)
{
    ui->setupUi(this);
    connect(ui->treeCampaign, &QTreeWidget::itemChanged, this, &ExportDialog::handleCampaignItemChanged);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &ExportDialog::close);
    connect(ui->btnExport, &QAbstractButton::clicked, this, &ExportDialog::runExport);
    connect(ui->grpMonsters, &QGroupBox::clicked, ui->grpMonsters, &QGroupBox::setEnabled);

    QList<CampaignObjectBase*> objectList = campaign.getChildObjects();
    for(CampaignObjectBase* rootObject : objectList)
    {
        if(rootObject)
        {
            QTreeWidgetItem* rootWidgetItem = createChildObject(rootObject, selectedItem);
            if(rootWidgetItem)
                ui->treeCampaign->addTopLevelItem(rootWidgetItem);
        }
    }

    ui->treeCampaign->expandAll();
}

ExportDialog::~ExportDialog()
{
    if(_worker)
        _worker->disconnect();

    if(_workerThread)
        _workerThread->disconnect();

    if(_workerThread)
    {
        _workerThread->quit();
        _workerThread->wait();
        delete _workerThread;
    }

    delete _worker;
    delete _waitingDlg;

    delete ui;
}

QTreeWidgetItem* ExportDialog::createChildObject(const CampaignObjectBase* childObject, const QUuid& selectedItem)
{
    if(!childObject)
        return nullptr;

    QTreeWidgetItem* childItem = new QTreeWidgetItem(QStringList(childObject->objectName()));
    childItem->setData(0, Qt::UserRole, QVariant::fromValue(childObject));
    setObjectIcon(childObject, childItem);
    childItem->setCheckState(0, Qt::Unchecked);
    if(!childItem)
        return nullptr;

    QList<CampaignObjectBase*> objectList = childObject->getChildObjects();
    for(CampaignObjectBase* nextObject : objectList)
    {
        if(nextObject)
        {
            QTreeWidgetItem* nextWidgetItem = createChildObject(nextObject, selectedItem);
            if(nextWidgetItem)
                childItem->addChild(nextWidgetItem);
        }
    }

    if(childObject->getID() == selectedItem)
    {
        setRecursiveChecked(childItem, true);
        checkObjectContent(childObject);
    }

    return childItem;
}

void ExportDialog::handleCampaignItemChanged(QTreeWidgetItem *item, int column)
{
    if((!item) || (column > 0))
        return;

    setRecursiveChecked(item, item->checkState(0) == Qt::Checked);
}

void ExportDialog::runExport()
{
    QString exportDirPath = QFileDialog::getExistingDirectory(this, QString("Select Export Directory"));
    if(exportDirPath.isEmpty())
        return;

    QDir exportDir(exportDirPath);
    if(!exportDir.exists())
        return;

    qDebug() << "[ExportDialog] Exporting to " << exportDirPath << ": " << (ui->btnFull->isChecked() ? QString("full export") : QString("assets only")) << ", monsters included: " << ui->grpMonsters->isChecked();

    _workerThread = new QThread(this);
    _worker = new ExportWorker(ui->treeCampaign->invisibleRootItem(), exportDir);
    _worker->moveToThread(_workerThread);
    connect(_workerThread, &QThread::finished, this, &ExportDialog::threadFinished);
    connect(_worker, &ExportWorker::workComplete, this, &ExportDialog::exportFinished);
    connect(this, &ExportDialog::startWork, _worker, &ExportWorker::doWork);

    if(!_waitingDlg)
        _waitingDlg = new DMHWaitingDialog(QString("Exporting..."), this);

    _workerThread->start();
    emit startWork();
    _waitingDlg->setModal(true);
    _waitingDlg->show();
}

void ExportDialog::exportFinished(bool success)
{
    Q_UNUSED(success);

    qDebug() << "[ExportDialog] Export completed, stopping dialog and thread.";

    if(_waitingDlg)
    {
        _waitingDlg->accept();
        _waitingDlg->deleteLater();
        _waitingDlg = nullptr;
    }

    if(_workerThread)
    {
        _workerThread->quit();
    }

    if(success)
    {
        QMessageBox::information(this, QString("DMHelper - Export Data"), QString("Export Completed!"));
        accept();
    }
    else
    {
        QMessageBox::critical(this, QString("DMHelper - Export Data"), QString("Export Failed!"));
    }
}

void ExportDialog::threadFinished()
{
    qDebug() << "[ExportDialog] Export thread finished.";

    if(_worker)
    {
        _worker->deleteLater();
        _worker = nullptr;
    }

    if(_workerThread)
    {
        _workerThread->deleteLater();
        _workerThread = nullptr;
    }
}

void ExportDialog::setRecursiveChecked(QTreeWidgetItem *item, bool checked)
{
    if(!item)
        return;

    item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
    for(int i = 0; i < item->childCount(); ++i)
        setRecursiveChecked(item->child(i), checked);

    refreshMonsters();
}

void ExportDialog::setObjectIcon(const CampaignObjectBase* baseObject, QTreeWidgetItem* widgetItem)
{
    switch(baseObject->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentparty.png"));
            break;
        case DMHelper::CampaignType_Combatant:
            {
                const Character* character = dynamic_cast<const Character*>(baseObject);
                bool isPC = ((character) && (character->isInParty()));
                widgetItem->setIcon(0, isPC ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png"));
            }
            break;
        case DMHelper::CampaignType_Map:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentmap.png"));
            break;
        case DMHelper::CampaignType_Text:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contenttextencounter.png"));
            break;
        case DMHelper::CampaignType_Battle:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentbattle.png"));
            break;
        case DMHelper::CampaignType_ScrollingText:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentscrollingtext.png"));
            break;
        case DMHelper::CampaignType_AudioTrack:
            {
                QString audioIcon(":/img/data/icon_soundboard.png");
                const AudioTrack* track = dynamic_cast<const AudioTrack*>(baseObject);
                if(track)
                {
                    if(track->getAudioType() == DMHelper::AudioType_Syrinscape)
                        audioIcon = QString(":/img/data/icon_syrinscape.png");
                    else if(track->getAudioType() == DMHelper::AudioType_Youtube)
                        audioIcon = QString(":/img/data/icon_playerswindow.png");
                }
                widgetItem->setIcon(0, QIcon(audioIcon));
            }
            break;
    }
}

void ExportDialog::refreshMonsters()
{
    while(ui->listMonsters->count() > 0)
        delete ui->listMonsters->takeItem(0);

    _monsters.clear();

    recursiveRefreshMonsters(ui->treeCampaign->invisibleRootItem());
}

void ExportDialog::recursiveRefreshMonsters(QTreeWidgetItem* widgetItem)
{
    if(!widgetItem)
        return;

    if(widgetItem->checkState(0) == Qt::Checked)
        checkObjectContent(widgetItem->data(0, Qt::UserRole).value<const CampaignObjectBase*>());

    for(int i = 0; i < widgetItem->childCount(); ++i)
    {
        recursiveRefreshMonsters(widgetItem->child(i));
    }
}

void ExportDialog::checkObjectContent(const CampaignObjectBase* object)
{
    if(!object)
        return;

    if(object->getObjectType() == DMHelper::CampaignType_Battle)
    {
        const EncounterBattle* battle = dynamic_cast<const EncounterBattle*>(object);
        if(!battle)
            return;

        BattleDialogModel* battleModel = battle->getBattleDialogModel();
        if(!battleModel)
            return;

        for(int i = 0; i < battleModel->getCombatantCount(); ++i)
        {
            BattleDialogModelCombatant* combatant = battleModel->getCombatant(i);
            if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Monster))
            {
                BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
                MonsterClass* monsterClass = monsterCombatant->getMonsterClass();
                if((monsterClass) && (!_monsters.contains(monsterClass->getName())))
                {
                    QListWidgetItem* listItem = new QListWidgetItem(QIcon(monsterClass->getIconPixmap(DMHelper::PixmapSize_Full)), monsterClass->getName());
                    ui->listMonsters->addItem(listItem);
                    _monsters.append(monsterClass->getName());
                }
            }
        }
    }
}

