#include "campaignexporter.h"
#include "character.h"
#include "map.h"
#include "audiotrack.h"
#include "combatantreference.h"
#include <QDebug>

CampaignExporter::CampaignExporter(Campaign& originalCampaign, QUuid exportId, QDir& exportDirectory) :
    _originalCampaign(originalCampaign),
    _exportId(exportId),
    _exportDirectory(exportDirectory),
    _exportCampaign(new Campaign("Export")),
    _exportDocument(new QDomDocument("DMHelperXML")),
    _exportedIds(),
    _valid(false)
{
    _exportCampaign->cleanupCampaign(false);
    _valid = populateExport();
}

CampaignExporter::~CampaignExporter()
{
    _exportCampaign->cleanupCampaign(false);

    delete _exportCampaign;
    delete _exportDocument;
}

Campaign& CampaignExporter::getExportCampaign()
{
    return *_exportCampaign;
}

QDomDocument& CampaignExporter::getExportDocument()
{
    return *_exportDocument;
}

bool CampaignExporter::isValid() const
{
    return _valid;
}

bool CampaignExporter::populateExport()
{
    QDomElement rootObject = _exportDocument->createElement("root");
    _exportDocument->appendChild(rootObject);
    QDomElement campaignElement = _exportCampaign->outputXML(*_exportDocument, rootObject, _exportDirectory, true);

    return addObjectTree(_exportId, *_exportDocument, campaignElement, _exportDirectory);
}

bool CampaignExporter::addObjectTree(QUuid exportId, QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    if(_exportedIds.contains(exportId))
        return false;

    // Get the object to export
    CampaignObjectBase* exportObject = _originalCampaign.getObjectById(exportId);
    if(!exportObject)
        return false;

    // Export the object
    QDomElement exportElement = exportObject->outputXML(doc, parent, targetDirectory, true);

    // Check for any further dependencies and export them as well
    addObjectAndChildrenIds(exportObject);
    return checkObjectTreeReferences(exportObject, doc, exportElement, targetDirectory);
}

bool CampaignExporter::checkObjectTreeReferences(CampaignObjectBase* exportObject, QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    if(!exportObject)
        return false;

    if(!checkObjectReferences(exportObject, doc, parent, targetDirectory))
        return false;

    QList<CampaignObjectBase*> childList = exportObject->getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        checkObjectTreeReferences(childList.at(i), doc, parent, targetDirectory);
    }

    return true;
}

bool CampaignExporter::checkObjectReferences(CampaignObjectBase* exportObject, QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    if(!exportObject)
        return false;

    if(exportObject->getObjectType() == DMHelper::CampaignType_Battle)
    {
        EncounterBattle* battle = dynamic_cast<EncounterBattle*>(exportObject);
        if(battle)
        {
            addObjectTree(battle->getAudioTrackId(), doc, parent, targetDirectory);
            CombatantGroupList combatants = battle->getCombatantsAllWaves();
            for(int i = 0; i < combatants.count(); ++i)
            {
                Combatant* combatant = combatants.at(i).second;
                if(combatant->getCombatantType() == DMHelper::CombatantType_Reference)
                {
                    CombatantReference* reference = dynamic_cast<CombatantReference*>(combatant);
                    if(reference)
                        addObjectTree(reference->getReferenceId(), doc, parent, targetDirectory);
                }
            }
        }
    }
    else if(exportObject->getObjectType() == DMHelper::CampaignType_Map)
    {
        Map* map = dynamic_cast<Map*>(exportObject);
        if(map)
            addObjectTree(map->getAudioTrackId(), doc, parent, targetDirectory);
    }

    return true;
}

void CampaignExporter::addObjectAndChildrenIds(CampaignObjectBase* object)
{
    if(_exportedIds.contains(object->getID()))
        return;

    _exportedIds.append(object->getID());

    QList<CampaignObjectBase*> childList = object->getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        addObjectAndChildrenIds(childList.at(i));
    }
}
