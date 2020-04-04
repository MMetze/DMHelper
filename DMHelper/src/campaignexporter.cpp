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
    /*
    bool result = addObjectForExport(_exportId);

    if(result)
    {
        QDomElement rootObject = _exportDocument->createElement("root");
        _exportDocument->appendChild(rootObject);
        _exportCampaign->outputXML(*_exportDocument, rootObject, _exportDirectory, true);
    }
    */

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

/*
void CampaignExporter::addEncounterScrollingText(Adventure& adventure, EncounterScrollingText& encounter)
{
    adventure.addEncounter(&encounter);
}

void CampaignExporter::addMap(Adventure& adventure, Map& map)
{
    QObject* oldParent = map.parent();
    adventure.addMap(&map);
    map.setParent(oldParent);
    addObjectForExport(map.getAudioTrackId());    }
}
*/

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

/*

bool CampaignExporter::addObjectForExport(QUuid exportId)
{
    if(exportId.isNull())
        return false;

    if(_exportCampaign->getObjectById(exportId) != nullptr)
        return false;

    // Get the object to export
    CampaignObjectBase* exportObject = _originalCampaign.getObjectById(exportId);
    if(!exportObject)
        return false;

    // Check the party
    Character* character = _originalCampaign.getCharacterById(exportId);
    if(character)
    {
        _exportCampaign->addCharacter(character);
        character->setParent(&_originalCampaign);
        return true;
    }

    // Check the settings
    Map* setting = _originalCampaign.getSettingById(exportId);
    if(setting)
    {
        _exportCampaign->addSetting(setting);
        setting->setParent(&_originalCampaign);
        addObjectForExport(setting->getAudioTrackId());
        return true;
    }

    // Check the NPCs
    Character* npc = _originalCampaign.getNPCById(exportId);
    if(npc)
    {
        _exportCampaign->addNPC(npc);
        npc->setParent(&_originalCampaign);
        return true;
    }

    // Check the audio tracks
    AudioTrack* track = _originalCampaign.getTrackById(exportId);
    if(track)
    {
        _exportCampaign->addTrack(track);
        track->setParent(&_originalCampaign);
        return true;
    }

    // Iterate through the adventures and check them and their contents
    for(int i = 0; i < _originalCampaign.getAdventureCount(); ++i)
    {
        Adventure* adventure = _originalCampaign.getAdventureByIndex(i);
        if(adventure)
        {
            if(adventure->getID() == exportId)
            {
                addWholeAdventure(*adventure);
                return true;
            }

            int j;
            for(j = 0; j < adventure->getEncounterCount(); ++j)
            {
                Encounter* encounter = adventure->getEncounterByIndex(j);
                if(encounter->getID() == exportId)
                {
                    Adventure* shell = addShellAdventure(*adventure);
                    switch(encounter->getType())
                    {
                        case DMHelper::EncounterType_Blank:
                            return false;
                        case DMHelper::EncounterType_Text:
                            addEncounterText(*shell, *dynamic_cast<EncounterText*>(encounter));
                            return true;
                        case DMHelper::EncounterType_Battle:
                            addEncounterBattle(*shell, *dynamic_cast<EncounterBattle*>(encounter));
                            return true;
                        case DMHelper::EncounterType_ScrollingText:
                            addEncounterScrollingText(*shell, *dynamic_cast<EncounterScrollingText*>(encounter));
                            return true;
                        default:
                            return false;
                    };
                }
            }

            for(j = 0; j < adventure->getMapCount(); ++j)
            {
                Map* map = adventure->getMapByIndex(j);
                if(map->getID() == exportId)
                {
                    Adventure* shell = addShellAdventure(*adventure);
                    addMap(*shell, *map);
                    return true;
                }
            }
        }
    }

    qDebug() << "[CampaignExporter]: ERROR - trying to export object with unknown ID: " << exportId;
    return false;
}

void CampaignExporter::addWholeAdventure(Adventure& adventure)
{
    Adventure* shell = addShellAdventure(adventure);

    int j;
    for(j = 0; j < adventure.getEncounterCount(); ++j)
    {
        Encounter* encounter = adventure.getEncounterByIndex(j);
        if(encounter)
        {
            QObject* oldParent = encounter->parent();
            switch(encounter->getType())
            {
                case DMHelper::EncounterType_Blank:
                    break;
                case DMHelper::EncounterType_Text:
                    addEncounterText(*shell, *dynamic_cast<EncounterText*>(encounter));
                    break;
                case DMHelper::EncounterType_Battle:
                    addEncounterBattle(*shell, *dynamic_cast<EncounterBattle*>(encounter));
                    break;
                case DMHelper::EncounterType_ScrollingText:
                    addEncounterScrollingText(*shell, *dynamic_cast<EncounterScrollingText*>(encounter));
                    break;
                default:
                    break;
            };
            encounter->setParent(oldParent);
        }
    }

    for(j = 0; j < adventure.getMapCount(); ++j)
    {
        addMap(*shell, *(adventure.getMapByIndex(j)));
    }
}

Adventure* CampaignExporter::addShellAdventure(Adventure& adventure)
{
    Adventure* shell = adventure.createShellClone();
    _shellAdventures.append(shell);
    _exportCampaign->addAdventure(shell);
    return shell;
}

void CampaignExporter::addEncounterText(Adventure& adventure, EncounterText& encounter)
{
    adventure.addEncounter(&encounter);
}

void CampaignExporter::addEncounterBattle(Adventure& adventure, EncounterBattle& encounter)
{
    adventure.addEncounter(&encounter);
    addObjectForExport(encounter.getAudioTrackId());

    CombatantGroupList combatants = encounter.getCombatantsAllWaves();
    for(int i = 0; i < combatants.count(); ++i)
    {
        Combatant* combatant = combatants.at(i).second;
        if(combatant->getType() == DMHelper::CombatantType_Reference)
        {
            CombatantReference* reference = dynamic_cast<CombatantReference*>(combatant);
            if(reference)
                addObjectForExport(reference->getReferenceId());
        }
    }
}

void CampaignExporter::addEncounterScrollingText(Adventure& adventure, EncounterScrollingText& encounter)
{
    adventure.addEncounter(&encounter);
}

void CampaignExporter::addMap(Adventure& adventure, Map& map)
{
    QObject* oldParent = map.parent();
    adventure.addMap(&map);
    map.setParent(oldParent);
    addObjectForExport(map.getAudioTrackId());
}
*/
