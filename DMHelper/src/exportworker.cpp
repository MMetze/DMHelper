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
#include "spellbook.h"
#include "spell.h"
#include <QTreeWidgetItem>
#include <QCryptographicHash>
#include <QDir>
#include <QDebug>


ExportWorker::ExportWorker(QTreeWidgetItem* widgetItem, const QDir& directory, bool fullExport, QObject *parent) :
    QObject(parent),
    _widgetItem(widgetItem),
    _spellList(),
    _monsterList(),
    _exportedSpells(),
    _exportedMonsters(),
    _directory(directory),
    _fullExport(fullExport)
{
}

void ExportWorker::doWork()
{
    bool result = false;

    if(!_directory.exists())
    {
        qDebug() << "[ExportWorker] Not exporting anything because " << _directory << " does not exist";
    }
    else
    {
        qDebug() << "[ExportWorker] Starting to export " << _widgetItem << " to " << _directory << "...";

        _exportedMonsters.clear();
        _exportedSpells.clear();

        if(_widgetItem)
            result = recursiveExport(_widgetItem, _directory);

        qDebug() << "[ExportWorker] Campaign tree export complete, exporting monsters...";
        for(QString monsterName : _monsterList)
        {
            exportMonster(monsterName, _directory);
        }

        qDebug() << "[ExportWorker] Monster export complete, exporting spells...";
        for(QString spellName : _spellList)
        {
            exportSpell(spellName, _directory);
        }

        qDebug() << "[ExportWorker] Export complete";
    }

    emit workComplete(result);
}

void ExportWorker::setSpellExports(QStringList spellList)
{
    _spellList = spellList;
}

void ExportWorker::setMonsterExports(QStringList monsterList)
{
    _monsterList = monsterList;
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
            exportFile(map->getFileName(), directory, true);
            break;
        }
        case DMHelper::CampaignType_Combatant:
        {
            const Combatant* combatant = dynamic_cast<const Combatant*>(object);
            qDebug() << "[ExportDialog] Exporting combatant: " << combatant->getName() << ", icon: " << combatant->getIcon();
            exportFile(combatant->getIcon(), directory, true);
            break;
        }
        case DMHelper::CampaignType_Text:
        {
            const EncounterText* textEncounter = dynamic_cast<const EncounterText*>(object);
            exportFile(textEncounter->getImageFile(), directory, true);
            break;
        }
        case DMHelper::CampaignType_AudioTrack:
        {
            const AudioTrack* track = dynamic_cast<const AudioTrack*>(object);
            if(track->getAudioType() == DMHelper::AudioType_File)
            {
                qDebug() << "[ExportDialog] Exporting track: " << track->getName() << ", file: " << track->getUrl();
                exportFile(track->getUrl().toString(), directory, true);
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
    for(i = 0; i < battleModel->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = battleModel->getCombatant(i);
        if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Monster))
        {
            BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
            exportMonster(monsterCombatant->getMonsterClass(), directory);
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
                exportFile(effect->getImageFile(), directory, true);
            }

            QList<CampaignObjectBase*> childObjects = effect->getChildObjects();
            for(CampaignObjectBase* childObject : childObjects)
            {
                BattleDialogModelEffect* childEffect = dynamic_cast<BattleDialogModelEffect*>(childObject);
                if((childEffect) && (!childEffect->getImageFile().isEmpty()))
                {
                    qDebug() << "[ExportDialog] Exporting child effect: " << childEffect->getName() << ", image: " << childEffect->getImageFile();
                    exportFile(childEffect->getImageFile(), directory, true);
                }
            }
        }
    }

    exportObjectAssets(battleModel->getMap(), directory);
}

void ExportWorker::exportMonster(const QString& monsterName, const QDir& directory)
{
    if((!Bestiary::Instance()) || (monsterName.isEmpty()))
        return;

    exportMonster(Bestiary::Instance()->getMonsterClass(monsterName), directory);
}

void ExportWorker::exportMonster(MonsterClass* monster, const QDir& directory)
{
    if((!Bestiary::Instance()) || (!monster) || (_exportedMonsters.contains(monster)))
        return;

    QString monsterIconFile = Bestiary::Instance()->findMonsterImage(monster->getName(), monster->getIcon());
    if(!monsterIconFile.isEmpty())
    {
        QString fullMonsterFile = Bestiary::Instance()->getDirectory().filePath(monsterIconFile);
        qDebug() << "[ExportDialog] Exporting monster: " << monster->getName() << ", icon: " << fullMonsterFile;
        exportFile(fullMonsterFile, directory, true);
    }

    _exportedMonsters.append(monster);
}

void ExportWorker::exportSpell(const QString& spellName, const QDir& directory)
{
    if((!Spellbook::Instance()) || (spellName.isEmpty()))
        return;

    Spell* spell = Spellbook::Instance()->getSpell(spellName);
    if((!spell) || (_exportedSpells.contains(spell)))
        return;

    QString spellToken = spell->getEffectTokenPath();
    if(!spellToken.isEmpty())
    {
        qDebug() << "[ExportDialog] Exporting spell: " << spell->getName() << ", token: " << spellToken;
        exportFile(spellToken, directory, true);
    }

    _exportedSpells.append(spell);
}

void ExportWorker::exportFile(const QString& filename, const QDir& directory, bool hashfileNaming)
{
    if((filename.isEmpty()) || !QFile::exists(filename))
        return;

    QString exportFileName;
    if(hashfileNaming)
    {
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
        exportFileName = directory.filePath(hashFileName);
    }
    else
    {

    }

    if((!exportFileName.isEmpty()) && (!QFile::exists(exportFileName)))
    {
        qDebug() << "[ExportDialog]     Copying file: " << filename << " to " << exportFileName;
        if(!QFile::copy(filename, exportFileName))
            qDebug() << "[ExportDialog]     ==> Copy FAILED!";
    }
}