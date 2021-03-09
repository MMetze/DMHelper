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
#include "map.h"
#include "character.h"
#include "bestiary.h"
#include <QUuid>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDebug>

ExportDialog::ExportDialog(const Campaign& campaign, const QUuid& selectedItem, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    _campaign(campaign),
    _selectedItem(selectedItem),
    _monsters()
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

    // TODO: Add waiting dialog

    recursiveExport(ui->treeCampaign->invisibleRootItem(), exportDir);
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

void ExportDialog::recursiveExport(QTreeWidgetItem* widgetItem, const QDir& directory)
{
    if(!widgetItem)
        return;

    if(widgetItem->checkState(0) == Qt::Checked)
        exportObjectAssets(widgetItem->data(0, Qt::UserRole).value<const CampaignObjectBase*>(), directory);

    for(int i = 0; i < widgetItem->childCount(); ++i)
    {
        recursiveExport(widgetItem->child(i), directory);
    }
}

void ExportDialog::exportObjectAssets(const CampaignObjectBase* object, const QDir& directory)
{
    if(!object)
        return;

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Map:
        {
            const Map* map = dynamic_cast<const Map*>(object);
            qDebug() << "[ExportDialog] Exporting map: " << map->getName() << ", file: " << map->getFileName();
            exportFile(map->getFileName(), directory);
            break;
        }
        case DMHelper::CampaignType_Combatant:
        {
            const Combatant* combatant = dynamic_cast<const Combatant*>(object);
            qDebug() << "[ExportDialog] Exporting combatant: " << combatant->getName() << ", icon: " << combatant->getIcon();
            exportFile(combatant->getIcon(), directory);
            break;
        }
        case DMHelper::CampaignType_Text:
        {
            //TODO : include with newer version and animated text!!!
            //const EncounterText* textEncounter = dynamic_cast<EncounterText*>(object);
            //exportFile(textEncounter->get)
            break;
        }
        case DMHelper::CampaignType_AudioTrack:
        {
            const AudioTrack* track = dynamic_cast<const AudioTrack*>(object);
            if(track->getAudioType() == DMHelper::AudioType_File)
            {
                qDebug() << "[ExportDialog] Exporting track: " << track->getName() << ", file: " << track->getUrl();
                exportFile(track->getUrl().toString(), directory);
            }
            break;
        }
        case DMHelper::CampaignType_Battle:
            exportBattle(dynamic_cast<const EncounterBattle*>(object), directory);
            break;
        default:
            break;
    }
}

void ExportDialog::exportBattle(const EncounterBattle* battle, const QDir& directory)
{
    if(!battle)
        return;

    BattleDialogModel* battleModel = battle->getBattleDialogModel();
    if(!battleModel)
        return;

    qDebug() << "[ExportDialog] Exporting battle: " << battle->getName();

    int i;
    QList<MonsterClass*> monsters;
    for(i = 0; i < battleModel->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = battleModel->getCombatant(i);
        if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Monster))
        {
            BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
            MonsterClass* monsterClass = monsterCombatant->getMonsterClass();
            if((monsterClass) && (!monsters.contains(monsterClass)))
            {
                QString monsterIconFile = Bestiary::Instance()->findMonsterImage(monsterClass->getName(), monsterClass->getIcon());
                if(!monsterIconFile.isEmpty())
                {
                    QString fullMonsterFile = Bestiary::Instance()->getDirectory().filePath(monsterIconFile);
                    qDebug() << "[ExportDialog] Exporting monster: " << monsterClass->getName() << ", icon: " << fullMonsterFile;
                    exportFile(fullMonsterFile, directory);
                    monsters.append(monsterClass);
                }
            }
        }
    }

    for(i = 0; i < battleModel->getEffectCount(); ++i)
    {
        BattleDialogModelEffect* effect = battleModel->getEffect(i);
        if(effect)
        {
            if(!effect->getImageFile().isEmpty())
            {
                qDebug() << "[ExportDialog] Exporting effect: " << effect->getName() << ", image: " << effect->getImageFile();
                exportFile(effect->getImageFile(), directory);
            }

            QList<CampaignObjectBase*> childObjects = effect->getChildObjects();
            for(CampaignObjectBase* childObject : childObjects)
            {
                BattleDialogModelEffect* childEffect = dynamic_cast<BattleDialogModelEffect*>(childObject);
                if((childEffect) && (!childEffect->getImageFile().isEmpty()))
                {
                    qDebug() << "[ExportDialog] Exporting child effect: " << childEffect->getName() << ", image: " << childEffect->getImageFile();
                    exportFile(childEffect->getImageFile(), directory);
                }
            }
        }
    }

    exportObjectAssets(battleModel->getMap(), directory);
}

void ExportDialog::exportFile(const QString& filename, const QDir& directory)
{
    if((filename.isEmpty()) || !QFile::exists(filename))
        return;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return;

    QByteArray readData = file.readAll();
    if(readData.size() <= 0)
        return;

    file.close();

    QByteArray byteHash = QCryptographicHash::hash(readData, QCryptographicHash::Md5);
    QString hashFileName = byteHash.toHex(0);
    QString exportFileName = directory.filePath(hashFileName);

    if(!QFile::exists(exportFileName))
    {
        qDebug() << "[ExportDialog]     Copying file: " << filename << " to " << exportFileName;
        if(!QFile::copy(filename, exportFileName))
            qDebug() << "[ExportDialog]     ==> Copy FAILED!";
    }
}
