#include "campaign.h"
#include "character.h"
#include "encounterfactory.h"
#include "map.h"
#include "party.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include "dmversion.h"
#include "bestiary.h"
#include "basicdateserver.h"
#include "soundboardgroup.h"
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
 * DONE open/close campaigns crashes
 * Update tree icons
 * Add conditions
 * Add create character from monster
 * CRASH at end of video
 * Main window context menus
 * Fix trace spam
 * Shortcuts
 * Add party view, rework character view, add party icon to battles/maps
 * Thin lines for brian
 *
 * BUGS
 *      Drag encounter to top level - crash
 *      Remove encounter
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
    _isValid(true),
    _soundboardGroups()
{
}

Campaign::~Campaign()
{
    qDeleteAll(_soundboardGroups);
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

    QString calendarName = element.attribute("calendar", QString("Gregorian"));
    if(BasicDateServer::Instance())
        BasicDateServer::Instance()->setActiveCalendar(calendarName);
    BasicDate inputDate(element.attribute("date",QString("")));
    setDate(inputDate);
    setTime(QTime::fromMSecsSinceStartOfDay(element.attribute("time",QString::number(0)).toInt()));

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

AudioTrack* Campaign::getTrackById(QUuid id)
{
    return dynamic_cast<AudioTrack*>(getObjectById(id));
}

QList<SoundboardGroup*> Campaign::getSoundboardGroups() const
{
    return _soundboardGroups;
}

void Campaign::addSoundboardGroup(SoundboardGroup* soundboardGroup)
{
    if(!soundboardGroup)
        return;

    _soundboardGroups.append(soundboardGroup);
}

void Campaign::removeSoundboardGroup(SoundboardGroup* soundboardGroup)
{
    if(!soundboardGroup)
        return;

    _soundboardGroups.removeOne(soundboardGroup);
    delete soundboardGroup;
}

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

    if(deleteAll)
    {
        CampaignObjectBase* child = findChild<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
        while(child)
        {
            child->setParent(nullptr);
            child->deleteLater();
            child = findChild<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
        }

        qDeleteAll(_soundboardGroups);
    }
    else
    {
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

    element.setAttribute("majorVersion", DMHelper::CAMPAIGN_MAJOR_VERSION);
    element.setAttribute("minorVersion", DMHelper::CAMPAIGN_MINOR_VERSION);
    element.setAttribute("calendar", BasicDateServer::Instance() ? BasicDateServer::Instance()->getActiveCalendarName() : QString());
    element.setAttribute("date", getDate().toStringDDMMYYYY());
    element.setAttribute("time", getTime().msecsSinceStartOfDay());

    QDomElement soundboardElement = doc.createElement("soundboard");
    for(SoundboardGroup* group : _soundboardGroups)
    {
        if(group)
        {
            QDomElement groupElement = doc.createElement("soundboardgroup");
            groupElement.setAttribute("groupname", group->getGroupName());
            for(AudioTrack* track : group->getTracks())
            {
                QDomElement trackElement = doc.createElement("soundboardtrack");
                trackElement.setAttribute("trackID", track->getID().toString());
                groupElement.appendChild(trackElement);
            }
            soundboardElement.appendChild(groupElement);
        }
    }
    element.appendChild(soundboardElement);
}

bool Campaign::belongsToObject(QDomElement& element)
{
    if(element.tagName() == QString("soundboard"))
        return true;
    else
        return CampaignObjectBase::belongsToObject(element);
}

void Campaign::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    /*
    // Compatibility mode for global expansion flags
    if(element.hasAttribute("partyExpanded"))
    {
        CampaignObjectBase* partyChild = findChild<CampaignObjectBase*>("Party", Qt::FindDirectChildrenOnly);
        if(partyChild)
            partyChild->setExpanded(static_cast<bool>(element.attribute("partyExpanded",QString::number(0)).toInt()));
    }
    if(element.hasAttribute("adventuresExpanded"))
    {
        CampaignObjectBase* adventuresChild = findChild<CampaignObjectBase*>("Adventures", Qt::FindDirectChildrenOnly);
        if(adventuresChild)
            adventuresChild->setExpanded(static_cast<bool>(element.attribute("adventuresExpanded",QString::number(0)).toInt()));
    }
    if(element.hasAttribute("worldSettingsExpanded"))
    {
        CampaignObjectBase* worldChild = findChild<CampaignObjectBase*>("Settings", Qt::FindDirectChildrenOnly);
        if(worldChild)
            worldChild->setExpanded(static_cast<bool>(element.attribute("worldSettingsExpanded",QString::number(0)).toInt()));
    }
    if(element.hasAttribute("worldNPCsExpanded"))
    {
        CampaignObjectBase* npcChild = findChild<CampaignObjectBase*>("Npcs", Qt::FindDirectChildrenOnly);
        if(npcChild)
            npcChild->setExpanded(static_cast<bool>(element.attribute("worldNPCsExpanded",QString::number(0)).toInt()));
    }
    */

    QDomElement soundboardElement = element.firstChildElement("soundboard");
    if(!soundboardElement.isNull())
    {
        QDomElement groupElement = soundboardElement.firstChildElement("soundboardgroup");
        while(!groupElement.isNull())
        {
            SoundboardGroup* group = new SoundboardGroup(groupElement.attribute("groupname"));
            QDomElement trackElement = groupElement.firstChildElement("soundboardtrack");
            while(!trackElement.isNull())
            {
                int trackIdInt = DMH_GLOBAL_INVALID_ID;
                QUuid trackId = parseIdString(trackElement.attribute("trackID"), &trackIdInt);
                AudioTrack* track = dynamic_cast<AudioTrack*>(getObjectById(trackId));
                if(track)
                    group->addTrack(track);

                trackElement = trackElement.nextSiblingElement("soundboardtrack");
            }
            addSoundboardGroup(group);
            groupElement = groupElement.nextSiblingElement("soundboardgroup");
        }
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
