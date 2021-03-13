#include "exportworker.h"
#include "character.h"
#include "audiotrack.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclass.h"
#include "encountertext.h"
#include "map.h"
#include "character.h"
#include "bestiary.h"
#include <QTreeWidgetItem>
#include <QCryptographicHash>
#include <QDir>
#include <QDebug>


ExportWorker::ExportWorker(QTreeWidgetItem* widgetItem, const QDir& directory, QObject *parent) :
    QObject(parent),
    _widgetItem(widgetItem),
    _directory(directory)
{
}

void ExportWorker::doWork()
{
    bool result = false;

    qDebug() << "[ExportWorker] Starting to export " << _widgetItem << " to " << _directory;

    if((_widgetItem) && (_directory.exists()))
        result = recursiveExport(_widgetItem, _directory);

    qDebug() << "[ExportWorker] Export complete";

    emit workComplete(result);
}

bool ExportWorker::recursiveExport(QTreeWidgetItem* widgetItem, const QDir& directory)
{
    if(!widgetItem)
        return false;

    qDebug() << "[ExportWorker] Exporting object " << widgetItem;

    if(widgetItem->checkState(0) == Qt::Checked)
        if(!exportObjectAssets(widgetItem->data(0, Qt::UserRole).value<const CampaignObjectBase*>(), directory))
            return false;

    for(int i = 0; i < widgetItem->childCount(); ++i)
    {
        if(!recursiveExport(widgetItem->child(i), directory))
            return false;
    }

    return true;
}

bool ExportWorker::exportObjectAssets(const CampaignObjectBase* object, const QDir& directory)
{
    if(!object)
        return false;

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
            const EncounterText* textEncounter = dynamic_cast<const EncounterText*>(object);
            exportFile(textEncounter->getImageFile(), directory);
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

    return true;
}

void ExportWorker::exportBattle(const EncounterBattle* battle, const QDir& directory)
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

void ExportWorker::exportFile(const QString& filename, const QDir& directory)
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

    QString path = directory.filePath(QString());
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
