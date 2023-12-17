#include "objectimportworker.h"
#include "campaign.h"
#include "map.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include "battledialogmodeleffect.h"
#include "bestiary.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDomDocument>
#include <QDebug>

ObjectImportWorker::ObjectImportWorker(QObject *parent) :
    QObject(parent),
    _campaign(nullptr),
    _parentObject(nullptr),
    _importFilename(),
    _assetPath(),
    _assetDir(),
    _replaceDuplicates(false),
    _importCampaign(nullptr),
    _duplicateObjects(),
    _importedBattles()
{
}

ObjectImportWorker::~ObjectImportWorker()
{
    delete _importCampaign;
}

bool ObjectImportWorker::setImportObject(Campaign* campaign, CampaignObjectBase* parentObject, const QString& importFilename, const QString& assetPath, bool replaceDuplicates)
{
    if((!campaign) || (!parentObject))
        return false;

    _campaign = campaign;
    _parentObject = parentObject;
    _importFilename = importFilename;
    _assetPath = assetPath;
    _assetDir = QDir(assetPath);
    _replaceDuplicates = replaceDuplicates;

    return true;
}

bool ObjectImportWorker::doWork()
{
    if((!_campaign) || (!_parentObject) || (_importFilename.isEmpty()) || (!_assetDir.exists()))
        return registerImportResult(false,
                                    QString("The required import parameters were not set prior to starting the import! Campaign: ") + _campaign->getName() + QString(", parent: ") + _parentObject->getName() + QString(", import file: ") + _importFilename + QString(", asset dir: ") + _assetDir.path(),
                                    QString("Invalid import started!"));

    QDomDocument doc("DMHelperXML");
    QFile file(_importFilename);
    if(!file.open(QIODevice::ReadOnly))
        return registerImportResult(false, QString("Not able to open the selected import file: ") + _importFilename);

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString contentError;
    int contentErrorLine = 0;
    int contentErrorColumn = 0;
    bool contentResult = doc.setContent(in.readAll(), &contentError, &contentErrorLine, &contentErrorColumn);

    file.close();

    QCoreApplication::processEvents();

    if(!contentResult)
        return registerImportResult(false, QString("Loading failed: error reading XML (line ") + QString::number(contentErrorLine) + QString(", column ") + QString::number(contentErrorColumn) + QString("): ") + contentError);

    QDomElement rootObject = doc.documentElement();
    if((rootObject.isNull()) || (rootObject.tagName() != "root"))
        return registerImportResult(false, QString("Loading failed: error reading XML - unable to find root entry"));

    QDomElement campaignElement = rootObject.firstChildElement(QString("campaign"));
    if(campaignElement.isNull())
        return registerImportResult(false, QString("Loading failed: error reading XML - unable to find campaign entry"));

    QFileInfo importFileInfo(_importFilename);
    QDir::setCurrent(importFileInfo.absolutePath());
    _importCampaign = new Campaign();

    Bestiary::Instance()->startBatchProcessing();
    _importCampaign->inputXML(campaignElement, true);
    _importCampaign->postProcessXML(campaignElement, true);
    Bestiary::Instance()->finishBatchProcessing();

    if(!_importCampaign->isValid())
        return registerImportResult(false, QString("The imported file contains a campaign with an invalid structure"));

    QCoreApplication::processEvents();

    int i;
    QList<CampaignObjectBase*> childList = _importCampaign->getChildObjects();
    for(i = 0; i < childList.count(); ++i)
    {
        importObject(childList.at(i), _parentObject, *_campaign, *_importCampaign);
    }

    postProcessBattles(campaignElement);

    return registerImportResult(true, QString());
}

void ObjectImportWorker::importObject(CampaignObjectBase* object, CampaignObjectBase* parentObject, Campaign& targetCampaign, Campaign& importCampaign)
{
    if((!object) || (!parentObject))
        return;

    emit updateStatus(QString("Importing Object: "), object->getName());

    CampaignObjectBase* existingObject = targetCampaign.getObjectById(object->getID());

    // First go through child objects
    QList<CampaignObjectBase*> childList = object->getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        importObject(childList.at(i), existingObject ? existingObject : object, targetCampaign, importCampaign);
    }

    importObjectAssets(object);

    // Either copy the entry or move it
    CampaignObjectBase* currentParent = dynamic_cast<CampaignObjectBase*>(object->parent());
    if(!currentParent)
        qDebug() << "[ObjectImportWorker] ERROR: Not able to find the parent for object: " << object->getName() << " with ID " << object->getID();

    if(existingObject)
    {
        existingObject->copyValues(object);
        // Keep track of all battle objects for later linking and full creation
        if(existingObject->getObjectType() == DMHelper::CampaignType_Battle)
            _importedBattles.append(dynamic_cast<EncounterBattle*>(existingObject));

        if((object->children().count() == 0) && (currentParent))
        {
            currentParent->removeObject(object);
            object->deleteLater();
        }
    }
    else
    {
        if(currentParent)
        {
            currentParent->removeObject(object);
            object->moveToThread(QApplication::instance()->thread());
            parentObject->addObject(object);
            // Keep track of all battle objects for later linking and full creation
            if(object->getObjectType() == DMHelper::CampaignType_Battle)
                _importedBattles.append(dynamic_cast<EncounterBattle*>(object));
        }
    }

    QCoreApplication::processEvents();

}

void ObjectImportWorker::importObjectAssets(CampaignObjectBase* object)
{
    // Go through, copy files and update object paths to new location
    if(!object)
        return;

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Map:
        {
            Map* map = dynamic_cast<Map*>(object);
            if(map)
            {
                // TODO: Layers import/export
                qDebug() << "[ObjectImportWorker] Importing map: " << map->getName() << ", file: " << map->getFileName();
                map->setFileName(importFile(map->getFileName()));
            }
            break;
        }
        case DMHelper::CampaignType_Combatant:
        {
            Combatant* combatant = dynamic_cast<Combatant*>(object);
            if(combatant)
            {
                qDebug() << "[ObjectImportWorker] Importing combatant: " << combatant->getName() << ", icon: " << combatant->getIcon();
                combatant->setIcon(importFile(combatant->getIcon()));
            }
            break;
        }
        case DMHelper::CampaignType_Text:
        {
            EncounterText* textEncounter = dynamic_cast<EncounterText*>(object);
            if(textEncounter)
            {
                qDebug() << "[ObjectImportWorker] Importing text entry: " << textEncounter->getName() << ", image file: " << textEncounter->getImageFile();
                textEncounter->setImageFile(importFile(textEncounter->getImageFile()));
            }
            break;
        }
        case DMHelper::CampaignType_AudioTrack:
        {
            AudioTrack* track = dynamic_cast<AudioTrack*>(object);
            if((track) && (track->getAudioType() == DMHelper::AudioType_File))
            {
                qDebug() << "[ObjectImportWorker] Importing track: " << track->getName() << ", file: " << track->getUrl();
                track->setUrl(QUrl(importFile(track->getUrl().toString())));
            }
            break;
        }
        case DMHelper::CampaignType_Battle:
        {
            EncounterBattle* battle = dynamic_cast<EncounterBattle*>(object);
            qDebug() << "[ObjectImportWorker] Importing battle: " << battle->getName();
            importBattle(battle);
            break;
        }
        case DMHelper::CampaignType_LinkedText:
        {
            // Todo: Markdown
            break;
        }
        default:
            break;
    }
}

QString ObjectImportWorker::importFile(const QString& filename)
{
    QFileInfo fileInfo(filename);
    QString filenameNoPath = fileInfo.fileName();
    if((_assetDir.exists(filenameNoPath)) && (_replaceDuplicates))
        QFile::remove(_assetDir.filePath(filenameNoPath));

    emit updateStatus(QString("Importing File: "), filename);

    QFile::copy(filename, _assetDir.filePath(filenameNoPath));

    QCoreApplication::processEvents();

    return _assetDir.filePath(filenameNoPath);
}

void ObjectImportWorker::importBattle(EncounterBattle* battle)
{
    if(!battle)
        return;

    BattleDialogModel* model = battle->getBattleDialogModel();
    if(!model)
        return;

    QList<BattleDialogModelEffect*> effects = model->getEffectList();
    for(BattleDialogModelEffect* effect : effects)
    {
        if(effect)
        {
            if(!effect->getImageFile().isEmpty())
            {
                qDebug() << "[ObjectImportWorker] Importing effect: " << effect->getName() << ", image: " << effect->getImageFile();
                effect->setImageFile(importFile(effect->getImageFile()));
            }

            QList<CampaignObjectBase*> childObjects = effect->getChildObjects();
            for(CampaignObjectBase* childObject : childObjects)
            {
                BattleDialogModelEffect* childEffect = dynamic_cast<BattleDialogModelEffect*>(childObject);
                if((childEffect) && (!childEffect->getImageFile().isEmpty()))
                {
                    qDebug() << "[ObjectImportWorker] Importing child effect: " << childEffect->getName() << ", image: " << childEffect->getImageFile();
                    childEffect->setImageFile(importFile(childEffect->getImageFile()));
                }
            }
        }
    }
}

bool ObjectImportWorker::registerImportResult(bool success, const QString& debugString, const QString& userMessage)
{
    qDebug() << "[ObjectImportWorker] " << debugString;
    emit workComplete(success, userMessage.isEmpty() ? debugString : userMessage);
    return success;
}

void ObjectImportWorker::postProcessBattles(const QDomElement& campaignElement)
{
    for(EncounterBattle* battle : _importedBattles)
    {
        if(battle)
        {
            emit updateStatus(QString("Importing Battle: "), battle->getName());

            QDomElement battleElement = findBattleElement(campaignElement, battle->getID().toString());
            if(!battleElement.isNull())
                battle->inputXMLBattle(battleElement, false); // Note: false to actually trigger loading
            else
                qDebug() << "[ObjectImportWorker] ERROR: could not post-process battle " << battle->getName() << ", ID: " << battle->getID();

            QCoreApplication::processEvents();
        }
    }
}

QDomElement ObjectImportWorker::findBattleElement(const QDomElement& element, const QString& idString)
{
    if((element.isNull()) || (idString.isEmpty()))
        return QDomElement();

    if((element.hasAttribute(QString("_baseID"))) && (element.attribute(QString("_baseID")) == idString))
        return element;

    QDomElement childElement = element.firstChildElement();
    while(!childElement.isNull())
    {
        QDomElement result = findBattleElement(childElement, idString);
        if(!result.isNull())
            return result;

        childElement = childElement.nextSiblingElement();
    }

    return QDomElement();
}
