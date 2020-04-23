#include "campaign.h"
#include "character.h"
#include "encounterfactory.h"
#include "map.h"
#include "party.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include "bestiary.h"
#include "basicdateserver.h"
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QDebug>

/*
 * XML strategy
 *
 * Input: initial creator (mainwindow) calls CampaignObjectFactory::createObject
 *          --> creates blank new object, then calls inputXML, alternatively directly combined in the constructor
 *          Each inputXML should as a final action call ::inputXML
 *          Each belongsToObject should return true for child elements it uses itself and otherwise return parentType::belongsToObject
 *          CampaignObjectBase::inputXML will for all child Elements check if belongsToObject, if not will call CampaignObjectCreator::createObject
 *
 *          CampaignObjectFactory needs to be expanded by other Factories
 *
 * Output: initial person calls CampaignObjectBase::outputXML
 *          Base implementation calls createOutputXML, then internalOutputXML, then outputXML on all direct children recursively, then appendChild
 *          createOutputXML should create the named output object
 *          internalOutputXML should first call ::internalOutputXML then output all necessary attributes and child elements
 *          generally, a class should NOT override outputXML
 *
 * TODO:
 *  Export/Import
 * Deal with clone functions in factories
 *  Text box hyperlinks
 *  Generic text controls
 * Get maps working again with registering/unregistering windows
 * Add factory support for old objects (adventures, adventure, maps, settings, notes, etc)
 *      Removed classes: adventure, encounter, monsterwidget, characterwidget, combatantwidget
 * New campaign structure
 * Where do battle text descriptions go? --> automatic sub-text
 *
 * Check versioning, warn for older file
 * Define location for importing things, especially dnd beyond characters
 * Widget activating...
 * define position to add a track (audiotrackedit.cpp line 47)
 * add/remove objects
 * open/close campaigns crashes
 *
 * Mainwindow: updatecampaigntree, contextmenu, handletreeitemchanged, handletreeitemselected, handletreestatechanged
 *
 * logging limits
 */

Campaign::Campaign(const QString& campaignName, QObject *parent) :
    CampaignObjectBase(campaignName, parent),
    _date(1,1,0),
    _time(0,0),
    _batchChanges(false),
    _changesMade(false),
    _dirtyMade(false),
    _isValid(true)
{
}

/*
Campaign::Campaign(const QDomElement& element, bool isImport, QObject *parent) :
    CampaignObjectBase(QString(), parent),
    _date(1,1,0),
    _time(0,0),
    _batchChanges(false),
    _changesMade(false),
    _dirtyMade(false),
    _isValid(false)
{
    inputXML(element, isImport);
//    postProcessXML(element, isImport);
}
*/

Campaign::~Campaign()
{
    //cleanupCampaign(true);
}

void Campaign::inputXML(const QDomElement &element, bool isImport)
{
    int majorVersion = element.attribute("majorVersion",QString::number(0)).toInt();
    int minorVersion = element.attribute("minorVersion",QString::number(0)).toInt();
    qDebug() << "[Campaign]    Campaign file version: " << majorVersion << "." << minorVersion;
    if(!isVersionCompatible(majorVersion, minorVersion))
    {
        qDebug() << "[Campaign]    ERROR: The camapaign file version is not compatible with this version of DM Helper.";
        return;
    }

    // TODO: WHY IS THIS NECESSARY?
    Bestiary::Instance()->startBatchProcessing();

    // int encounterCount = 0;
    // int mapCount = 0;

    QString calendarName = element.attribute("calendar", QString("Gregorian"));
    if(BasicDateServer::Instance())
        BasicDateServer::Instance()->setActiveCalendar(calendarName);
    BasicDate inputDate(element.attribute("date",QString("")));
    setDate(inputDate);
    setTime(QTime::fromMSecsSinceStartOfDay(element.attribute("time",QString::number(0)).toInt()));

    //inputCampaignChildren(element, isImport);

    CampaignObjectBase::inputXML(element, isImport);
    Bestiary::Instance()->finishBatchProcessing();

    // TODO: add back in some kind of object counting
    // Sum up all the elements loaded. The +2 is for the campaign object itself and the notes object
    //int totalElements = characters.count() + settings.count() + npcs.count() + adventures.count() + tracks.count() + encounterCount + mapCount + 2;

    qDebug() << "[Campaign] Loaded campaign """ << getName();
    //qDebug() << "[Campaign] Loaded campaign """ << _name << """ containing " << totalElements << " elements";
    //qDebug() << "           Date: " << _date.toStringDDMMYYYY() << ", Time: " << _time;
    //qDebug() << "           Party: " << characters.count() << " characters";
    //qDebug() << "           Settings: " << settings.count();
    //qDebug() << "           NPCs: " << npcs.count();
    //qDebug() << "           Adventures: " << adventures.count();
    //qDebug() << "               Encounters: " << encounterCount;
    //qDebug() << "               Maps: " << mapCount;
    //qDebug() << "           Audio Tracks: " << tracks.count();

    validateCampaignIds();
}

void Campaign::postProcessXML(const QDomElement &element, bool isImport)
{
    internalPostProcessXML(element, isImport);
    CampaignObjectBase::postProcessXML(element, isImport);

    // DEPRECATED v2.0
    // This is compatibility mode only to avoid an "unknown" node when importing an old-style campaign
    EncounterText* notesObject = dynamic_cast<EncounterText*>(searchDirectChildrenByName("Notes"));
    if(!notesObject)
        return;

    QDomElement notesElement = element.firstChildElement("notes");
    if(notesElement.isNull())
        return;

    QDomElement childElement = notesElement.firstChildElement("encounter");
    if((childElement.isNull()) || (childElement.attribute("name") != QString("")))
        return;

    QDomNode childNode = childElement.firstChild();
    while(!childNode.isNull())
    {
        if(childNode.isCDATASection())
        {
            QDomCDATASection cdata = childNode.toCDATASection();
            notesObject->setText(cdata.data());
            return;
        }
        childNode = childNode.nextSibling();
    }
}

int Campaign::getObjectType() const
{
    return DMHelper::CampaignType_Campaign;
}

/*
void Campaign::resolveReferences()
{
    for( int charIt = 0; charIt < characters.size(); ++charIt )
    {
        characters.at(charIt)->resolveReferences();
    }

    for( int advIt = 0; advIt < adventures.size(); ++advIt )
    {
        adventures.at(advIt)->resolveReferences();
    }

    for( int settingIt = 0; settingIt < settings.size(); ++settingIt )
    {
        settings.at(settingIt)->resolveReferences();
    }

    for( int npcIt = 0; npcIt < npcs.size(); ++npcIt )
    {
        npcs.at(npcIt)->resolveReferences();
    }

    for( int trackIt = 0; trackIt < tracks.size(); ++trackIt )
    {
        tracks.at(trackIt)->resolveReferences();
    }
}
*/

/*
void Campaign::widgetActivated(QWidget* widget)
{
    Q_UNUSED(widget);
    qDebug() << "[Campaign]: ERROR widget activated on Campaign object. This should never happen!";
}

void Campaign::widgetDeactivated(QWidget* widget)
{
    Q_UNUSED(widget);
    qDebug() << "[Campaign]: ERROR widget deactivated on Campaign object. This should never happen!";
}
*/

void Campaign::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
    _dirtyMade = false;
}

void Campaign::endBatchChanges()
{
    _batchChanges = false;
    if(_changesMade)
        emit changed();

    if((_dirtyMade) || (_changesMade))
        emit dirty();
}

/*
QString Campaign::getName() const
{
    return _name;
}

void Campaign::setName(const QString& campaignName)
{
    if(_name != campaignName)
    {
        _name = campaignName;
        emit dirty();
    }
}
*/

/*
Encounter* Campaign::getNotes() const
{
    return _notes;
}

int Campaign::getCharacterCount()
{
    return characters.count();
}
*/

Character* Campaign::getCharacterById(QUuid id)
{
    Character* character = dynamic_cast<Character*>(getObjectById(id));
    if(!character)
        return nullptr;

    if(character->isInParty())
        return character;
    else
        return nullptr;
}

const Character* Campaign::getCharacterById(QUuid id) const
{
    const Character* character = dynamic_cast<const Character*>(getObjectById(id));
    if(!character)
        return nullptr;

    if(character->isInParty())
        return character;
    else
        return nullptr;
}

Character* Campaign::getCharacterByDndBeyondId(int id)
{
    QList<Party*> partyList = findChildren<Party*>();

    for(int p = 0; p < partyList.count(); ++p)
    {
        QList<Character*> characterList = partyList.at(p)->findChildren<Character*>();
        for(int i = 0; i < characterList.count(); ++i)
        {
            if(characterList.at(i)->getDndBeyondID() == id)
                return characterList.at(i);
        }
    }

    return nullptr;
}

Character* Campaign::getCharacterOrNPCByDndBeyondId(int id)
{
    QList<Character*> characterList = findChildren<Character*>();

    for(int i = 0; i < characterList.count(); ++i)
    {
        if(characterList.at(i)->getDndBeyondID() == id)
            return characterList.at(i);
    }

    return nullptr;
}

/*
Character* Campaign::getCharacterByIndex(int index)
{
    if((index < 0)||(index >= characters.size()))
        return nullptr;

    return characters.at(index);
}

QUuid Campaign::addCharacter(Character* character)
{
    if(!character)
        return QUuid();

    character->setParent(this);
    characters.append(character);
    connect(character,SIGNAL(dirty()),this,SLOT(handleInternalDirty()));
    connect(character,SIGNAL(changed()),this,SLOT(handleInteralChange()));
    handleInteralChange();
    handleInternalDirty();
    return character->getID();
}

Character* Campaign::removeCharacter(QUuid id)
{
    Character* character = getCharacterById(id);
    if(character)
    {
        if(characters.removeOne(character))
        {
            handleInteralChange();
            handleInternalDirty();
            return character;
        }
    }

    return nullptr;
}
*/

QList<Character*> Campaign::getActiveCharacters()
{
    QList<Character*> actives;

    QList<Party*> partyList = findChildren<Party*>();
    for(int p = 0; p < partyList.count(); ++p)
    {
        QList<Character*> characterList = partyList.at(p)->findChildren<Character*>();
        for(int i = 0; i < characterList.count(); ++i)
        {
            if(characterList.at(i)->getActive())
                actives.append(characterList.at(i));
        }
    }

    return actives;
}

/*
QList<Combatant*> Campaign::getActiveCombatants()
{
    QList<Combatant*> actives;

    for(int i = 0; i < characters.count(); ++i)
    {
        if(characters.at(i)->getActive())
            actives.append(characters.at(i));
    }

    return actives;
}

int Campaign::getAdventureCount()
{
    return adventures.count();
}
*/

/*
Adventure* Campaign::getAdventureById(QUuid id)
{
    return dynamic_cast<Adventure*>(getObjectById(id));
}
*/

/*
Adventure* Campaign::getAdventureByIndex(int index)
{
    if((index < 0)||(index >= adventures.size()))
        return nullptr;

    return adventures.at(index);
}


QUuid Campaign::addAdventure(Adventure* adventure)
{
    if(!adventure)
        return QUuid();

    adventure->setParent(this);
    adventures.append(adventure);
    connect(adventure,SIGNAL(changed()),this,SLOT(handleInteralChange()));
    connect(adventure,SIGNAL(dirty()),this,SLOT(handleInternalDirty()));
    handleInteralChange();
    handleInternalDirty();
    return adventure->getID();
}

Adventure* Campaign::removeAdventure(QUuid id)
{
    Adventure* adventure = getAdventureById(id);
    if(adventure)
    {
        if(adventures.removeOne(adventure))
        {
            handleInteralChange();
            handleInternalDirty();
            return adventure;
        }
    }

    return nullptr;
}

int Campaign::getSettingCount()
{
    return settings.count();
}

Map* Campaign::getSettingById(QUuid id)
{
    for(int i = 0; i < settings.count(); ++i)
    {
        if(settings.at(i)->getID() == id)
            return settings.at(i);
    }

    return nullptr;
}

Map* Campaign::getSettingByIndex(int index)
{
    if((index < 0)||(index >= settings.size()))
        return nullptr;

    return settings.at(index);
}

QUuid Campaign::addSetting(Map* setting)
{
    if(!setting)
        return QUuid();

    setting->setParent(this);
    settings.append(setting);
    connect(setting,SIGNAL(dirty()),this,SLOT(handleInternalDirty()));
    connect(setting,SIGNAL(changed()),this,SLOT(handleInteralChange()));
    handleInteralChange();
    handleInternalDirty();
    return setting->getID();
}

Map* Campaign::removeSetting(QUuid id)
{
    Map* setting = getSettingById(id);
    if(setting)
    {
        if(settings.removeOne(setting))
        {
            handleInteralChange();
            handleInternalDirty();
            return setting;
        }
    }

    return nullptr;
}

int Campaign::getNPCCount()
{
    return npcs.count();
}
*/

Character* Campaign::getNPCById(QUuid id)
{
    Character* character = dynamic_cast<Character*>(getObjectById(id));
    if(!character)
        return nullptr;

    if(character->isInParty())
        return nullptr;
    else
        return character;
}

const Character* Campaign::getNPCById(QUuid id) const
{
    const Character* character = dynamic_cast<const Character*>(getObjectById(id));
    if(!character)
        return nullptr;

    if(character->isInParty())
        return nullptr;
    else
        return character;
}

/*
Character* Campaign::getNPCByIndex(int index)
{
    if((index < 0)||(index >= npcs.size()))
        return nullptr;

    return npcs.at(index);
}

QUuid Campaign::addNPC(Character* npc)
{
    if(!npc)
        return QUuid();

    npc->setParent(this);
    npcs.append(npc);
    connect(npc,SIGNAL(dirty()),this,SLOT(handleInternalDirty()));
    connect(npc,SIGNAL(changed()),this,SLOT(handleInteralChange()));
    handleInteralChange();
    handleInternalDirty();
    return npc->getID();
}

Character* Campaign::removeNPC(QUuid id)
{
    Character* npc = getNPCById(id);
    if(npc)
    {
        if(npcs.removeOne(npc))
        {
            handleInteralChange();
            handleInternalDirty();
            return npc;
        }
    }

    return nullptr;
}

int Campaign::getTrackCount()
{
    return tracks.count();
}
*/

AudioTrack* Campaign::getTrackById(QUuid id)
{
    return dynamic_cast<AudioTrack*>(getObjectById(id));
}

/*
AudioTrack* Campaign::getTrackByIndex(int index)
{
    if((index < 0)||(index >= tracks.size()))
        return nullptr;

    return tracks.at(index);
}

QUuid Campaign::addTrack(AudioTrack* track)
{
    if(!track)
        return QUuid();

    tracks.append(track);
    connect(track,SIGNAL(dirty()),this,SLOT(handleInternalDirty()));
    connect(track,SIGNAL(changed()),this,SLOT(handleInteralChange()));
    handleInteralChange();
    handleInternalDirty();
    return track->getID();
}

AudioTrack* Campaign::removeTrack(QUuid id)
{
    AudioTrack* track = getTrackById(id);
    if(track)
    {
        if(tracks.removeOne(track))
        {
            handleInteralChange();
            handleInternalDirty();
            return track;
        }
    }

    return nullptr;
}
*/

/*
CampaignObjectBase* Campaign::getObjectbyId(QUuid id)
{
    if(id.isNull())
        return nullptr;

    CampaignObjectBase* result;

    result = getCharacterById(id);
    if(result)
        return result;

    for(int i = 0; i < adventures.count(); ++i)
    {
        if(adventures.at(i)->getID() == id)
            return adventures.at(i);

        result = adventures.at(i)->getEncounterById(id);
        if(result)
            return result;

        result = adventures.at(i)->getMapById(id);
        if(result)
            return result;
    }

    result = getSettingById(id);
    if(result)
        return result;

    result = getNPCById(id);
    if(result)
        return result;

    result = getTrackById(id);
    if(result)
        return result;

    return nullptr;
}
*/

/*
QUuid Campaign::getUuidFromIntId(int intId) const
{
    if(intId == DMH_GLOBAL_INVALID_ID)
        return QUuid();

    int i, j;
    for(i = 0; i < characters.count(); ++i)
    {
        if(characters.at(i)->getIntID() == intId)
            return characters.at(i)->getID();
    }

    for(i = 0; i < adventures.count(); ++i)
    {
        Adventure* adventure = adventures.at(i);
        if(adventure->getIntID() == intId)
            return adventure->getID();

        for(j = 0; j < adventure->getEncounterCount(); ++j)
        {
            if(adventure->getEncounterByIndex(j)->getIntID() == intId)
                return adventure->getEncounterByIndex(j)->getID();
        }

        for(j = 0; j < adventure->getMapCount(); ++j)
        {
            if(adventure->getMapByIndex(j)->getIntID() == intId)
                return adventure->getMapByIndex(j)->getID();
        }
    }

    for(i = 0; i < settings.count(); ++i)
    {
        if(settings.at(i)->getIntID() == intId)
            return settings.at(i)->getID();
    }

    for(i = 0; i < npcs.count(); ++i)
    {
        if(npcs.at(i)->getIntID() == intId)
            return npcs.at(i)->getID();
    }

    for(i = 0; i < tracks.count(); ++i)
    {
        if(tracks.at(i)->getIntID() == intId)
            return tracks.at(i)->getID();
    }

    qDebug() << "[Campaign] WARNING: unable to find matching object for Integer ID " << intId;
    return QUuid();
}
*/

/*
QList<CampaignObjectBase*> Campaign::getObjectsByType(int campaignType)
{
    QList<CampaignObjectBase*> result;
    switch(campaignType)
    {
        case DMHelper::CampaignType_Party:
            {
                QList<Party*> partyList = findChildren<Party*>();
                for(int i = 0; i < partyList.count(); ++i)
                    result.append(partyList.at(i));
                break;
            }
        case DMHelper::CampaignType_Encounter:
            {
                QList<Encounter*> encounterList = findChildren<Encounter*>();
                for(int i = 0; i < encounterList.count(); ++i)
                    result.append(encounterList.at(i));
                break;
            }
        case DMHelper::CampaignType_Combatant:
            {
                QList<Combatant*> combatantList = findChildren<Combatant*>();
                for(int i = 0; i < combatantList.count(); ++i)
                    result.append(combatantList.at(i));
                break;
            }
        case DMHelper::CampaignType_Map:
            {
                QList<Map*> mapList = findChildren<Map*>();
                for(int i = 0; i < mapList.count(); ++i)
                    result.append(mapList.at(i));
                break;
            }
        case DMHelper::CampaignType_Battle:
        case DMHelper::CampaignType_BattleContent:
        case DMHelper::CampaignType_Campaign:
        case DMHelper::CampaignType_Base:
        case DMHelper::CampaignType_Placeholder:
        default:
            break;
    }

    return result;
}
*/

/*
bool Campaign::getPartyExpanded() const
{
    return _partyExpanded;
}

void Campaign::setPartyExpanded(bool expanded)
{
    if(_partyExpanded != expanded)
    {
        _partyExpanded = expanded;
        emit dirty();
    }
}

bool Campaign::getAdventuresExpanded() const
{
    return _adventuresExpanded;
}

void Campaign::setAdventuresExpanded(bool expanded)
{
    if(_adventuresExpanded != expanded)
    {
        _adventuresExpanded = expanded;
        emit dirty();
    }
}

bool Campaign::getWorldExpanded() const
{
    return _worldExpanded;
}

void Campaign::setWorldExpanded(bool expanded)
{
    if(_worldExpanded != expanded)
    {
        _worldExpanded = expanded;
        emit dirty();
    }
}

bool Campaign::getWorldSettingsExpanded() const
{
    return _worldSettingsExpanded;
}

void Campaign::setWorldSettingsExpanded(bool expanded)
{
    if(_worldSettingsExpanded != expanded)
    {
        _worldSettingsExpanded = expanded;
        emit dirty();
    }
}

bool Campaign::getWorldNPCsExpanded() const
{
    return _worldNPCsExpanded;
}

void Campaign::setWorldNPCsExpanded(bool expanded)
{
    if(_worldNPCsExpanded != expanded)
    {
        _worldNPCsExpanded = expanded;
        emit dirty();
    }
}
*/

BasicDate Campaign::getDate() const
{
    return _date;
}

QTime Campaign::getTime() const
{
    return _time;
}

bool Campaign::isValid() const
{
    return _isValid;
}

void Campaign::cleanupCampaign(bool deleteAll)
{
    if(_batchChanges)
        endBatchChanges();

//    delete _notes;
//    _notes = nullptr;

    if(deleteAll)
    {
//        qDeleteAll(characters);
//        qDeleteAll(adventures);
//        qDeleteAll(settings);
//        qDeleteAll(npcs);
//        qDeleteAll(tracks);
//        qDeleteAll(_contents);
        CampaignObjectBase* child = findChild<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
        while(child)
        {
            child->setParent(nullptr);
            child->deleteLater();
            child = findChild<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
        }
    }
    else
    {
//        characters.clear();
//        adventures.clear();
//        settings.clear();
//        npcs.clear();
//        tracks.clear();

        //_contents.clear();
        qDebug() << "[Campaign] TODO";
    }
}

void Campaign::setDate(const BasicDate& date)
{
    if(date.isValid() && (_date != date))
    {
        _date = date;
        emit dateChanged(_date);
        emit dirty();
    }
}

void Campaign::setTime(const QTime& time)
{
    if(_time != time)
    {
        _time = time;
        emit timeChanged(_time);
        emit dirty();
    }
}

bool Campaign::validateCampaignIds()
{
    QList<QUuid> knownIds;

    _isValid = validateSingleId(knownIds, this);

    /*validResult = validResult && validateSingleId(knownIds, _notes);

    int i;
    for(i = 0; i < characters.count(); ++i)
        validResult = validResult && validateSingleId(knownIds, characters.at(i));

    for(i = 0; i < adventures.count(); ++i)
    {
        validResult = validResult && validateSingleId(knownIds, adventures.at(i));

        if(adventures.at(i))
        {
            int j;
            for(j = 0; j < adventures.at(i)->getEncounterCount(); ++j)
                validResult = validResult && validateSingleId(knownIds, adventures.at(i)->getEncounterByIndex(j));

            for(j = 0; j < adventures.at(i)->getMapCount(); ++j)
                validResult = validResult && validateSingleId(knownIds, adventures.at(i)->getMapByIndex(j));
        }
    }

    for(i = 0; i < settings.count(); ++i)
        validResult = validResult && validateSingleId(knownIds, settings.at(i));

    for(i = 0; i < npcs.count(); ++i)
        validResult = validResult && validateSingleId(knownIds, npcs.at(i));

    for(i = 0; i < tracks.count(); ++i)
        validResult = validResult && validateSingleId(knownIds, tracks.at(i));
        */

    qDebug() << "[Campaign] IDs validated result = " << _isValid << ",  " << knownIds.count() << " unique IDs";

    return _isValid;
}

void Campaign::handleInternalChange()
{
    if(_batchChanges)
        _changesMade = true;
    else
        emit changed();
}

void Campaign::handleInternalDirty()
{
    if(_batchChanges)
        _dirtyMade = true;
    else
        emit dirty();
}

QDomElement Campaign::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("campaign");
}

void Campaign::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);

    //element.setAttribute("name", getName());
    element.setAttribute("majorVersion", DMHelper::CAMPAIGN_MAJOR_VERSION);
    element.setAttribute("minorVersion", DMHelper::CAMPAIGN_MINOR_VERSION);
    element.setAttribute("calendar", BasicDateServer::Instance() ? BasicDateServer::Instance()->getActiveCalendarName() : QString());
    element.setAttribute("date", getDate().toStringDDMMYYYY());
    element.setAttribute("time", getTime().msecsSinceStartOfDay());
}

void Campaign::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    // Compatibility mode for global expansion flags
    if(element.hasAttribute("partyExpanded"))
    {
        CampaignObjectBase* partyChild = findChild<CampaignObjectBase*>("Party", Qt::FindDirectChildrenOnly);
        if(partyChild)
            partyChild->setExpanded(static_cast<bool>(element.attribute("partyExpanded",QString::number(0)).toInt()));
    }
    if(element.hasAttribute("adventuresExpanded"))
    {
        CampaignObjectBase* partyChild = findChild<CampaignObjectBase*>("Adventures", Qt::FindDirectChildrenOnly);
        if(partyChild)
            partyChild->setExpanded(static_cast<bool>(element.attribute("adventuresExpanded",QString::number(0)).toInt()));
    }
    if(element.hasAttribute("worldSettingsExpanded"))
    {
        CampaignObjectBase* partyChild = findChild<CampaignObjectBase*>("Settings", Qt::FindDirectChildrenOnly);
        if(partyChild)
            partyChild->setExpanded(static_cast<bool>(element.attribute("worldSettingsExpanded",QString::number(0)).toInt()));
    }
    if(element.hasAttribute("worldNPCsExpanded"))
    {
        CampaignObjectBase* partyChild = findChild<CampaignObjectBase*>("Npcs", Qt::FindDirectChildrenOnly);
        if(partyChild)
            partyChild->setExpanded(static_cast<bool>(element.attribute("worldNPCsExpanded",QString::number(0)).toInt()));
    }

    CampaignObjectBase::internalPostProcessXML(element, isImport);
}

bool Campaign::validateSingleId(QList<QUuid>& knownIds, CampaignObjectBase* baseObject)
{
    if(!baseObject)
        return false;

    bool result = false;
    if(knownIds.contains(baseObject->getID()))
    {
        qCritical() << "[Campaign] duplicated campaign id: " << baseObject->getID();
        result = false;
    }
    else
    {
        knownIds.append(baseObject->getID());
        //qSort(knownIds.begin(), knownIds.end());
        // TODO: check if this works
        std::sort(knownIds.begin(), knownIds.end());
        result = true;
    }

    QList<CampaignObjectBase*> childList = baseObject->getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        if(!validateSingleId(knownIds, childList.at(i)))
            result = false;
    }

    return result;
}

bool Campaign::isVersionCompatible(int majorVersion, int minorVersion) const
{
    if(majorVersion > DMHelper::CAMPAIGN_MAJOR_VERSION)
        return false;

    if((majorVersion == DMHelper::CAMPAIGN_MAJOR_VERSION) &&
       (minorVersion > DMHelper::CAMPAIGN_MINOR_VERSION))
        return false;

    return true;
}
