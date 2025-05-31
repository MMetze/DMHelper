#include "exportworker.h"
#include "campaign.h"
#include "characterv2.h"
#include "audiotrack.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclassv2.h"
#include "encountertext.h"
#include "map.h"
#include "bestiary.h"
#include "spellbook.h"
#include "spell.h"
#include "dmversion.h"
#include <QTreeWidgetItem>
#include <QCryptographicHash>
#include <QDir>
#include <QDebug>


ExportWorker::ExportWorker(Campaign& campaign, QTreeWidgetItem* widgetItem, QDir& directory, const QString& exportName, QObject *parent) :
    QObject(parent),
    _campaign(campaign),
    _widgetItem(widgetItem),
    _exportName(exportName),
    _campaignExport(false),
    _referencesExport(false),
    _soundboardExport(false),
    _monstersExport(false),
    _spellsExport(false),
    _spellList(),
    _monsterList(),
    _exportedSpells(),
    _exportedMonsters(),
    _exportedFiles(),
    _directory(directory)
{
}

void ExportWorker::doWork()
{
    bool result = tryToDoWork();
    emit workComplete(result);
}

void ExportWorker::setExportFlags(bool campaign, bool references, bool soundboard, bool monsters, bool spells)
{
    _campaignExport = campaign;
    _referencesExport = references;
    _soundboardExport = soundboard;
    _monstersExport = monsters;
    _spellsExport = spells;
}

void ExportWorker::setSpellExports(QStringList spellList)
{
    _spellList = spellList;
}

void ExportWorker::setMonsterExports(QStringList monsterList)
{
    _monsterList = monsterList;
}

void ExportWorker::recursiveExport(QTreeWidgetItem* widgetItem, QDomDocument &doc, QDomElement &parentElement, QDir& directory)
{
    if(!widgetItem)
        return;

    qDebug() << "[ExportWorker] Exporting object " << widgetItem;

    QDomElement objectElement;

    if(widgetItem->checkState(0) == Qt::Checked)
    {
        CampaignObjectBase* object = widgetItem->data(0, Qt::UserRole).value<CampaignObjectBase*>();
        if(object)
        {
            if(_campaignExport)
                objectElement = object->outputXML(doc, parentElement, directory, true);

            if((!objectElement.isNull()) && (_referencesExport))
                exportObjectAssets(object, directory, doc, objectElement);
        }
    }

    for(int i = 0; i < widgetItem->childCount(); ++i)
    {
        recursiveExport(widgetItem->child(i), doc, (objectElement.isNull() ? parentElement : objectElement), directory);
    }
}

void ExportWorker::exportObjectAssets(const CampaignObjectBase* object, QDir& directory, QDomDocument &doc, QDomElement &element)
{
    if(!object)
        return;

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Map:
        {
            const Map* map = dynamic_cast<const Map*>(object);
            if(map)
            {
                qDebug() << "[ExportWorker] Exporting map: " << map->getName() << ", file: " << map->getFileName();
                exportFile(map->getFileName(), directory, element, QString("filename"), false);
            }
            break;
        }
        case DMHelper::CampaignType_Combatant:
        {
            const Combatant* combatant = dynamic_cast<const Combatant*>(object);
            if(combatant)
            {
                qDebug() << "[ExportWorker] Exporting combatant: " << combatant->getName() << ", icon: " << combatant->getIconFile();
                exportFile(combatant->getIconFile(), directory, element, QString("icon"), false);
            }
            break;
        }
        case DMHelper::CampaignType_Text:
        {
            const EncounterText* textEncounter = dynamic_cast<const EncounterText*>(object);
            if(textEncounter)
            {
                qDebug() << "[ExportWorker] Exporting text entry: " << textEncounter->getName();
                exportFile(textEncounter->getImageFile(), directory, element, QString("imageFile"), false);
            }
            break;
        }
        case DMHelper::CampaignType_AudioTrack:
        {
            const AudioTrack* track = dynamic_cast<const AudioTrack*>(object);
            if((track) && (track->getAudioType() == DMHelper::AudioType_File))
            {
                qDebug() << "[ExportWorker] Exporting track: " << track->getName() << ", file: " << track->getUrl();
                QString exportedFile = exportFile(track->getUrl().toString(), directory, element, QString(), false);
                if((!exportedFile.isEmpty()) && (!element.isNull()))
                {
                    element.removeChild(element.firstChildElement("url"));
                    QDomElement urlElement = doc.createElement("url");
                    QDomCDATASection urlData = doc.createCDATASection(exportedFile);
                    urlElement.appendChild(urlData);
                    element.appendChild(urlElement);

                }
            }
            break;
        }
        case DMHelper::CampaignType_Battle:
            exportBattle(dynamic_cast<const EncounterBattle*>(object), directory, element);
            break;
        case DMHelper::CampaignType_LinkedText:
            // TODO: Markdown
            break;
        default:
            break;
    }
}

void ExportWorker::exportBattle(const EncounterBattle* battle, QDir& directory, QDomElement &battleElement)
{
    if(!battle)
        return;

    BattleDialogModel* battleModel = battle->getBattleDialogModel();
    if(!battleModel)
        return;

    qDebug() << "[ExportWorker] Exporting battle: " << battle->getName();

    int i;
    QDomElement effectsElement = battleElement.firstChildElement("effects");
    for(i = 0; i < battleModel->getEffectCount(); ++i)
    {
        BattleDialogModelEffect* effect = battleModel->getEffect(i);
        if(effect)
        {
            QDomElement effectElement = findEffectElement(effectsElement, effect);
            if(!effect->getImageFile().isEmpty())
            {
                qDebug() << "[ExportWorker] Exporting effect: " << effect->getName() << ", image: " << effect->getImageFile();
                exportFile(effect->getImageFile(), directory, effectElement, QString("filename"), false);
            }

            QList<CampaignObjectBase*> childObjects = effect->getChildObjects();
            for(CampaignObjectBase* childObject : childObjects)
            {
                BattleDialogModelEffect* childEffect = dynamic_cast<BattleDialogModelEffect*>(childObject);
                if((childEffect) && (!childEffect->getImageFile().isEmpty()))
                {
                    qDebug() << "[ExportWorker] Exporting child effect: " << childEffect->getName() << ", image: " << childEffect->getImageFile();
                    QDomElement childElement = findEffectElement(effectElement, childEffect);
                    exportFile(childEffect->getImageFile(), directory, childElement, QString("filename"), false);
                }
            }
        }
    }
}

void ExportWorker::exportMonster(QDomDocument &doc, QDomElement& bestiaryElement, const QString& monsterName, QDir& directory)
{
    if((!Bestiary::Instance()) || (monsterName.isEmpty()))
        return;

    exportMonster(doc, bestiaryElement, Bestiary::Instance()->getMonsterClass(monsterName), directory);
}

void ExportWorker::exportMonster(QDomDocument &doc, QDomElement& bestiaryElement, MonsterClassv2* monster, QDir& directory)
{
    if((!Bestiary::Instance()) || (!monster) || (_exportedMonsters.contains(monster)))
        return;

    QDomElement monsterElement;

    if(!bestiaryElement.isNull())
    {
        monsterElement = doc.createElement("element");
        monster->outputXML(doc, monsterElement, directory, true);
    }

    QString monsterIconFile = Bestiary::Instance()->findMonsterImage(monster->getStringValue("name"), monster->getIcon());
    if(!monsterIconFile.isEmpty())
    {
        QString fullMonsterFile = Bestiary::Instance()->getDirectory().filePath(monsterIconFile);
        qDebug() << "[ExportWorker] Exporting monster: " << monster->getStringValue("name") << ", icon: " << fullMonsterFile;
        exportFile(fullMonsterFile, directory, monsterElement, QString("icon"), false);
    }

    if((!bestiaryElement.isNull()) && (!monsterElement.isNull()))
        bestiaryElement.appendChild(monsterElement);

    _exportedMonsters.append(monster);
}

void ExportWorker::exportSpell(QDomDocument &doc, QDomElement& spellbookElement, const QString& spellName, QDir& directory)
{
    if((!Spellbook::Instance()) || (spellName.isEmpty()))
        return;

    Spell* spell = Spellbook::Instance()->getSpell(spellName);
    if((!spell) || (_exportedSpells.contains(spell)))
        return;

    QDomElement spellElement;
    QDomElement effectElement;

    if(!spellbookElement.isNull())
    {
        QDomElement spellElement = doc.createElement("spell");
        spell->outputXML(doc, spellElement, directory, true);
        effectElement = spellElement.firstChildElement("effect");
    }

    QString spellToken = spell->getEffectTokenPath();
    if(!spellToken.isEmpty())
    {
        qDebug() << "[ExportWorker] Exporting spell: " << spell->getName() << ", token: " << spellToken;
        exportFile(spellToken, directory, effectElement, QString("token"), false);
    }

    if((!spellbookElement.isNull()) && (!spellElement.isNull()))
        spellbookElement.appendChild(spellElement);

    _exportedSpells.append(spell);
}

QString ExportWorker::exportFile(const QString& filename, const QDir& directory, QDomElement& element, const QString& fileAttribute, bool hashfileNaming)
{
    if((filename.isEmpty()) || !QFile::exists(filename))
        return QString();

    QString exportFileName;
    if(hashfileNaming)
        exportFileName = exportHashedFile(filename, directory);
    else
        exportFileName = exportDirectFile(filename, directory);

    if(exportFileName.isEmpty())
        return QString();

    qDebug() << "[ExportWorker]     Copying file: " << filename << " to " << exportFileName;
    if((QFile::exists(exportFileName)) || (QFile::copy(filename, exportFileName)))
    {
        if((!element.isNull()) && (!fileAttribute.isEmpty()))
        {
            QFileInfo fileInfo(exportFileName);
            element.setAttribute(fileAttribute, fileInfo.fileName());
        }
        return exportFileName;
    }
    else
    {
        qDebug() << "[ExportWorker]     ==> Copy FAILED!";
        return QString();
    }
}

bool ExportWorker::tryToDoWork()
{
    if(!_directory.exists())
    {
        qDebug() << "[ExportWorker] Not exporting anything because " << _directory << " does not exist";
        return false;
    }

    QString exportFileName(_exportName + QString(".xml"));
    qDebug() << "[ExportWorker] Starting to export to " << _directory << " for " << _exportName << "...";

    _exportedMonsters.clear();
    _exportedSpells.clear();

    QDomDocument doc("DMHelperXML");
    QDomElement root;
    QDomElement campaignElement;

    if(_campaignExport)
    {
        root = doc.createElement("root");
        doc.appendChild(root);

        campaignElement = _campaign.outputXML(doc, root, _directory, true);
        if(campaignElement.isNull())
        {
            qDebug() << "[ExportWorker] ERROR: Not exporting anything because unabled to export campaign object!";
            return false;
        }

        campaignElement.setAttribute("name", _exportName);

        if(!_soundboardExport)
        {
            QDomElement soundboardElement = campaignElement.firstChildElement("soundboard");
            campaignElement.removeChild(soundboardElement);
        }
    }

    if(_widgetItem)
        recursiveExport(_widgetItem, doc, campaignElement, _directory);

    if(_campaignExport)
        root.appendChild(campaignElement);

    if((_monstersExport) && (_monsterList.count() > 0))
    {
        QDomElement bestiaryElement = doc.createElement("bestiary");
        bestiaryElement.setAttribute("majorversion", DMHelper::BESTIARY_MAJOR_VERSION);
        bestiaryElement.setAttribute("minorversion", DMHelper::BESTIARY_MINOR_VERSION);

        for(QString monsterName : _monsterList)
        {
            exportMonster(doc, bestiaryElement, monsterName, _directory);
        }

        root.appendChild(bestiaryElement);
    }

    if((_spellsExport) && (_spellList.count() > 0))
    {
        QDomElement spellbookElement = doc.createElement("spellbook");
        spellbookElement.setAttribute("majorversion", DMHelper::SPELLBOOK_MAJOR_VERSION);
        spellbookElement.setAttribute("minorversion", DMHelper::SPELLBOOK_MINOR_VERSION);

        for(QString spellName : _spellList)
        {
            exportSpell(doc, spellbookElement, spellName, _directory);
        }

        root.appendChild(spellbookElement);
    }

    if(_campaignExport)
    {
        qDebug() << "[ExportWorker] Exporting campaign tree...";
        QFile file(_directory.absoluteFilePath(exportFileName));
        if(!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "[ExportWorker] Unable to open campaign file for writing: " << _directory.absoluteFilePath(exportFileName);
            qDebug() << "       Error " << file.error() << ": " << file.errorString();
            return false;
        }

        QTextStream ts(&file);
        ts.setEncoding(QStringConverter::Utf8);
        ts << doc.toString();

        file.close();
        qDebug() << "[ExportWorker] Campaign tree export complete.";
    }

    qDebug() << "[ExportWorker] Export completed successfully!";

    return true;
}

QString ExportWorker::exportHashedFile(const QString& filename, const QDir& directory)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return QString();

    QByteArray readData = file.readAll();
    if(readData.size() <= 0)
        return QString();

    file.close();

    QString path = directory.filePath(QString());
    QByteArray byteHash = QCryptographicHash::hash(readData, QCryptographicHash::Md5);
    QString hashFileName = byteHash.toHex(0);
    QString exportFileName = directory.filePath(hashFileName);

    return exportFileName;
}

QString ExportWorker::exportDirectFile(const QString& filename, const QDir& directory)
{
    if(_exportedFiles.contains(filename))
        return _exportedFiles.value(filename);

    QFileInfo fileInfo(filename);
    QString exportFileName(fileInfo.fileName());
    int n = 0;
    while((directory.exists(exportFileName)) && (n < 1000))
    {
        ++n;
        exportFileName = fileInfo.baseName() + QString("_") + QString::number(n) + QString(".") + fileInfo.completeSuffix();
    }

    if(n < 1000)
    {
        exportFileName = directory.filePath(exportFileName);
        _exportedFiles.insert(filename, exportFileName);
        return exportFileName;
    }

    return QString();
}

QDomElement ExportWorker::findEffectElement(QDomElement& parentElement, BattleDialogModelEffect* effect)
{
    if(effect)
    {
        QDomElement childElement = parentElement.firstChildElement("battleeffect");
        while(!childElement.isNull())
        {
            if(QUuid(childElement.attribute(QString("_baseID"))) == effect->getID())
                return childElement;

            childElement = childElement.nextSiblingElement("battleffect");
        }
    }

    return QDomElement();
}
