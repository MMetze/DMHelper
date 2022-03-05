#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "campaign.h"
#include "dmconstants.h"
#include "character.h"
#include "map.h"
#include "audiotrack.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelcharacter.h"
#include "dmhwaitingdialog.h"
#include "exportworker.h"
#include "bestiary.h"
#include "monsterclass.h"
#include "spellbook.h"
#include "spell.h"
#include "selectstringdialog.h"
#include <QUuid>
#include <QFileDialog>
#include <QThread>
#include <QMessageBox>
#include <QDebug>

ExportDialog::ExportDialog(Campaign& campaign, const QUuid& selectedItem, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    _campaign(campaign),
    _selectedItem(selectedItem),
    _monsters(),
    _spells(),
    _characters(),
    _workerThread(nullptr),
    _worker(nullptr),
    _waitingDlg(nullptr)
{
    ui->setupUi(this);
    connect(ui->treeCampaign, &QTreeWidget::itemClicked, this, &ExportDialog::handleCampaignItemChanged);
    connect(ui->btnExport, &QAbstractButton::clicked, this, &ExportDialog::runExport);
    connect(ui->btnAddMonsters, &QAbstractButton::clicked, this, &ExportDialog::addMonsters);
    connect(ui->btnAddSpells, &QAbstractButton::clicked, this, &ExportDialog::addSpells);

    ui->edtExportName->setText(campaign.getName()); // + "_" + QDateTime::currentDateTime().toString(Qt::ISODate));

    QList<CampaignObjectBase*> objectList = campaign.getChildObjects();
    for(CampaignObjectBase* rootObject : objectList)
    {
        if(rootObject)
        {
            QTreeWidgetItem* rootWidgetItem = createChildObject(rootObject);
            if(rootWidgetItem)
                ui->treeCampaign->addTopLevelItem(rootWidgetItem);
        }
    }

    checkItem(_selectedItem);

    refreshMonsters();
    checkCharacters();

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

QTreeWidgetItem* ExportDialog::createChildObject(CampaignObjectBase* childObject)
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
            QTreeWidgetItem* nextWidgetItem = createChildObject(nextObject);
            if(nextWidgetItem)
                childItem->addChild(nextWidgetItem);
        }
    }

    return childItem;
}

void ExportDialog::handleCampaignItemChanged(QTreeWidgetItem *item, int column)
{
    if((!item) || (column > 0))
        return;

    for(int i = 0; i < item->childCount(); ++i)
        setRecursiveChecked(item->child(i), item->checkState(0) == Qt::Checked);

    if(item->checkState(0) == Qt::Checked)
    {
        checkObjectContent(item->data(0, Qt::UserRole).value<CampaignObjectBase*>());
        setRecursiveParentChecked(item->parent());
    }

    checkCharacters();
    refreshMonsters();
}

void ExportDialog::addMonsters()
{
    if(!Bestiary::Instance())
        return;

    SelectStringDialog dlg;
    dlg.resize(width() / 2, height() * 9 / 10);

    QList<QString> monsters = Bestiary::Instance()->getMonsterList();
    for(QString monster : monsters)
    {
        dlg.addEntry(monster);
    }

    int result = dlg.exec();
    if(result != QDialog::Accepted)
        return;

    QStringList selectedMonsters = dlg.getSelection();
    for(QString selected : selectedMonsters)
    {
        addMonster(Bestiary::Instance()->getMonsterClass(selected));
    }
}

void ExportDialog::addSpells()
{
    if(!Spellbook::Instance())
        return;

    SelectStringDialog dlg;
    dlg.resize(width() / 2, height() * 9 / 10);

    QList<QString> spells = Spellbook::Instance()->getSpellList();
    for(QString spellName : spells)
    {
        dlg.addEntry(spellName);
    }

    int result = dlg.exec();
    if(result != QDialog::Accepted)
        return;

    QStringList selectedSpells = dlg.getSelection();
    for(QString selected : selectedSpells)
    {
        addSpell(Spellbook::Instance()->getSpell(selected));
    }
}

void ExportDialog::runExport()
{
    QString exportDirPath = QFileDialog::getExistingDirectory(this, QString("Select Export Directory"));
    if(exportDirPath.isEmpty())
        return;

    QDir exportDir(exportDirPath);
    if(!exportDir.exists())
        return;

    QString exportFileName(ui->edtExportName->text() + QString(".xml"));
    if((ui->chkCampaignFile->isChecked()) && (exportDir.exists(exportFileName)))
    {
        QMessageBox::StandardButton result = QMessageBox::question(nullptr,
                                                                   QString("Export file exists"),
                                                                   QString("The target export file ") + exportDir.absoluteFilePath(exportFileName) + QString(" already exists. Do you want to overwrite the existing file?"));
        if(result == QMessageBox::No)
        {
            qDebug() << "[ExportDialog] Not exporting anything because " << exportDir.absoluteFilePath(exportFileName) << " already exists and should not be overwritten.";
            return;
        }
    }


    qDebug() << "[ExportDialog] Exporting to " << exportDirPath << " as " << ui->edtExportName->text() << ", monsters included: " << ui->grpMonsters->isChecked() << ", spells included: " << ui->grpSpells->isChecked();

    _workerThread = new QThread(this);
    _worker = new ExportWorker(_campaign, ui->treeCampaign->invisibleRootItem(), exportDir, ui->edtExportName->text());
    _worker->setExportFlags(ui->chkCampaignFile->isChecked(),
                            ui->chkReferences->isChecked(),
                            ui->chkSoundboard->isChecked(),
                            ui->grpMonsters->isChecked(),
                            ui->grpSpells->isChecked());
    _worker->setSpellExports(_spells);
    _worker->setMonsterExports(_monsters);
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

    Qt::CheckState targetState = checked ? Qt::Checked : Qt::Unchecked;

    if(item->checkState(0) == targetState)
        return;

    item->setCheckState(0, targetState);
    if(item->checkState(0) == Qt::Checked)
        checkObjectContent(item->data(0, Qt::UserRole).value<CampaignObjectBase*>());

    for(int i = 0; i < item->childCount(); ++i)
        setRecursiveChecked(item->child(i), checked);
}

void ExportDialog::setRecursiveParentChecked(QTreeWidgetItem *item)
{
    if((!item) || (item == ui->treeCampaign->invisibleRootItem()))
        return;

    item->setCheckState(0, Qt::Checked);
    checkObjectContent(item->data(0, Qt::UserRole).value<CampaignObjectBase*>());
    setRecursiveParentChecked(item->parent());
}

void ExportDialog::setObjectIcon(CampaignObjectBase* baseObject, QTreeWidgetItem* widgetItem)
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

void ExportDialog::checkCharacters()
{
    while(!_characters.isEmpty())
    {
        checkItem(_characters.takeFirst());
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
        checkObjectContent(widgetItem->data(0, Qt::UserRole).value<CampaignObjectBase*>());

    for(int i = 0; i < widgetItem->childCount(); ++i)
        recursiveRefreshMonsters(widgetItem->child(i));
}

void ExportDialog::checkObjectContent(CampaignObjectBase* object)
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

        checkItem(battleModel->getMap());

        for(int i = 0; i < battleModel->getCombatantCount(); ++i)
        {
            BattleDialogModelCombatant* combatant = battleModel->getCombatant(i);
            if(combatant)
            {
                if(combatant->getCombatantType() == DMHelper::CombatantType_Monster)
                {
                    BattleDialogModelMonsterBase* monster = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
                    if(monster)
                       addMonster(monster->getMonsterClass());
                }
                else if(combatant->getCombatantType() == DMHelper::CombatantType_Character)
                {
                    BattleDialogModelCharacter* character = dynamic_cast<BattleDialogModelCharacter*>(combatant);
                    if(character)
                        addCharacter(character->getCharacter());
                }
            }
        }
    }
}

void ExportDialog::checkItem(CampaignObjectBase* object)
{
    if(object)
        checkItem(object->getID());
}

void ExportDialog::checkItem(QUuid id)
{
    QTreeWidgetItem *item = findItem(ui->treeCampaign->invisibleRootItem(), id);
    if(item)
    {
        setRecursiveChecked(item, true);
        setRecursiveParentChecked(item->parent());
    }
}

QTreeWidgetItem* ExportDialog::findItem(QTreeWidgetItem *item, QUuid id)
{
    if(!item)
        return nullptr;

    CampaignObjectBase* object = item->data(0, Qt::UserRole).value<CampaignObjectBase*>();
    if((object) && (object->getID() == id))
        return item;

    for(int i = 0; i < item->childCount(); ++i)
    {
        QTreeWidgetItem* result = findItem(item->child(i), id);
        if(result)
            return result;
    }

    return nullptr;
}

void ExportDialog::addCharacter(Character* character)
{
    if((!character) || _characters.contains(character))
        return;

    _characters.append(character);
}

void ExportDialog::addMonster(MonsterClass* monsterClass)
{
    if((!monsterClass) || (_monsters.contains(monsterClass->getName())))
        return;

    QListWidgetItem* listItem = new QListWidgetItem(QIcon(monsterClass->getIconPixmap(DMHelper::PixmapSize_Full)), monsterClass->getName());
    ui->listMonsters->addItem(listItem);
    _monsters.append(monsterClass->getName());
}

void ExportDialog::addSpell(Spell* spell)
{
    if((!spell) || (_spells.contains(spell->getName())))
        return;

    QPixmap spellPixmap;
    if(!spell->getEffectTokenPath().isEmpty())
        spellPixmap.load(spell->getEffectTokenPath());

    QListWidgetItem* listItem = new QListWidgetItem(QIcon(spellPixmap), spell->getName());
    ui->listSpells->addItem(listItem);
    _spells.append(spell->getName());
}
