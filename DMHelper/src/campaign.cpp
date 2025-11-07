#include "campaign.h"
#include "characterv2.h"
#include "encounterfactory.h"
#include "map.h"
#include "party.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include "dmversion.h"
#include "bestiary.h"
#include "basicdateserver.h"
#include "soundboardgroup.h"
#include "campaignobjectfactory.h"
#include "overlaycounter.h"
#include "overlayfear.h"
#include "overlaytimer.h"
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
    _date(1, 1, 0),
    _time(0, 0),
    _notes(),
    _fearCount(0),
    _ruleset(),
    _batchChanges(false),
    _changesMade(false),
    _dirtyMade(false),
    _isValid(true),
    _soundboardGroups()
{
    connect(&_ruleset, &Ruleset::dirty, this, &Campaign::dirty);
}

Campaign::~Campaign()
{
    qDeleteAll(_soundboardGroups);
}

void Campaign::inputXML(const QDomElement &element, bool isImport)
{
    int majorVersion = element.attribute("majorVersion", QString::number(0)).toInt();
    int minorVersion = element.attribute("minorVersion", QString::number(0)).toInt();
    qDebug() << "[Campaign]    Campaign file version: " << majorVersion << "." << minorVersion;
    if(!isVersionCompatible(majorVersion, minorVersion))
    {
        qDebug() << "[Campaign]    ERROR: The campaign file version is not compatible with this version of DMHelper.";
        return;
    }

    // Load the ruleset; without this we can't load the rest of the campaign
    if(!_ruleset.isInitialized())
        preloadRulesetXML(element, isImport);

    // Configure the campaign object factories based on the ruleset
    CampaignObjectFactory::configureFactories(_ruleset, majorVersion, minorVersion);

    QString calendarName = element.attribute("calendar", QString("Gregorian"));
    if(BasicDateServer::Instance())
        BasicDateServer::Instance()->setActiveCalendar(calendarName);
    BasicDate inputDate(element.attribute("date", QString("")));
    setDate(inputDate);
    setTime(QTime::fromMSecsSinceStartOfDay(element.attribute("time", QString::number(0)).toInt()));

    // TODO: Remove special case for Daggerheart and add campaign-specific data storage(?)
    _fearCount = element.attribute("fear", QString::number(0)).toInt();

    // Load the bulk of the campaign contents
    CampaignObjectBase::inputXML(element, isImport);

    QDomElement notesElement = element.firstChildElement(QString("notes"));
    if(!notesElement.isNull())
    {
        QDomCDATASection notesData = notesElement.firstChild().toCDATASection();
        setNotes(notesData.data());
    }

    // Load the overlays
    loadOverlayXML(element.firstChildElement(QString("overlays")));

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

    CampaignObjectFactory::configureFactories(_ruleset, DMHelper::CAMPAIGN_MAJOR_VERSION, DMHelper::CAMPAIGN_MINOR_VERSION);
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

void Campaign::preloadRulesetXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    QDomElement rulesetElement = element.firstChildElement(QString("ruleset"));
    if(!rulesetElement.isNull())
        _ruleset.inputXML(rulesetElement, isImport);
    else
        _ruleset.setValues(RuleFactory::Instance()->getRulesetTemplate(RuleFactory::DEFAULT_RULESET_NAME));
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

Characterv2* Campaign::getCharacterById(QUuid id)
{
    Characterv2* character = dynamic_cast<Characterv2*>(getObjectById(id));
    if(!character)
        return nullptr;

    if(character->isInParty())
        return character;
    else
        return nullptr;
}

const Characterv2* Campaign::getCharacterById(QUuid id) const
{
    const Characterv2* character = dynamic_cast<const Characterv2*>(getObjectById(id));
    if(!character)
        return nullptr;

    if(character->isInParty())
        return character;
    else
        return nullptr;
}

Characterv2* Campaign::getCharacterByDndBeyondId(int id)
{
    QList<Party*> partyList = findChildren<Party*>();

    for(int p = 0; p < partyList.count(); ++p)
    {
        QList<Characterv2*> characterList = partyList.at(p)->findChildren<Characterv2*>();
        for(int i = 0; i < characterList.count(); ++i)
        {
            if(characterList.at(i)->getDndBeyondID() == id)
                return characterList.at(i);
        }
    }

    return nullptr;
}

Characterv2* Campaign::getCharacterOrNPCByDndBeyondId(int id)
{
    QList<Characterv2*> characterList = findChildren<Characterv2*>();

    for(int i = 0; i < characterList.count(); ++i)
    {
        if(characterList.at(i)->getDndBeyondID() == id)
            return characterList.at(i);
    }

    return nullptr;
}

QList<Characterv2*> Campaign::getActiveCharacters()
{
    QList<Characterv2*> actives;

    QList<Party*> partyList = findChildren<Party*>();
    for(int p = 0; p < partyList.count(); ++p)
    {
        QList<Characterv2*> characterList = partyList.at(p)->findChildren<Characterv2*>();
        for(int i = 0; i < characterList.count(); ++i)
        {
            if(characterList.at(i)->getBoolValue(QString("active")))
                actives.append(characterList.at(i));
        }
    }

    return actives;
}

Characterv2* Campaign::getNPCById(QUuid id)
{
    Characterv2* character = dynamic_cast<Characterv2*>(getObjectById(id));
    if(!character)
        return nullptr;

    if(character->isInParty())
        return nullptr;
    else
        return character;
}

const Characterv2* Campaign::getNPCById(QUuid id) const
{
    const Characterv2* character = dynamic_cast<const Characterv2*>(getObjectById(id));
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

QList<Overlay*> Campaign::getOverlays()
{
    return _overlays;
}

int Campaign::getOverlayCount() const
{
    return _overlays.size();
}

int Campaign::getOverlayIndex(Overlay* overlay)
{
    if(!overlay)
        return -1;

    return _overlays.indexOf(overlay);
}

bool Campaign::addOverlay(Overlay* overlay)
{
    if(!overlay)
        return false;

    overlay->setParent(this);
    overlay->setCampaign(this);
    _overlays.append(overlay);

    emit overlaysChanged();
    return true;
}

bool Campaign::removeOverlay(Overlay* overlay)
{
    if(!overlay)
        return false;

    int index = _overlays.indexOf(overlay);
    if(index == -1)
        return false;
    _overlays.removeAt(index);
    delete overlay;

    emit overlaysChanged();
    return true;
}

bool Campaign::moveOverlay(int from, int to)
{
    if((from < 0) || (from >= _overlays.size()) || (to < 0) || (to >= _overlays.size()) || (from == to))
        return false;

    _overlays.swapItemsAt(from, to);

    emit overlaysChanged();
    return true;
}

void Campaign::clearOverlays()
{
    while(!_overlays.isEmpty())
    {
        Overlay* overlay = _overlays.takeFirst();
        delete overlay;
    }

    emit overlaysChanged();
}

BasicDate Campaign::getDate() const
{
    return _date;
}

QTime Campaign::getTime() const
{
    return _time;
}

int Campaign::getFearCount() const
{
    return _fearCount;
}

Ruleset& Campaign::getRuleset()
{
    return _ruleset;
}

const Ruleset& Campaign::getRuleset() const
{
    return _ruleset;
}

bool Campaign::isValid() const
{
    return _isValid;
}

QStringList Campaign::getNotes() const
{
    return _notes;
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

void Campaign::setNotes(const QString& notes)
{
    QStringList newNotes = notes.split(QString("\n"));
    if(_notes != newNotes)
    {
        _notes = newNotes;
        emit dirty();
    }
}

void Campaign::addNote(const QString& note)
{
    _notes.append(note);
    emit dirty();
}

void Campaign::setFearCount(int fearCount)
{
    if((fearCount < 0) || (fearCount == _fearCount))
        return;

    _fearCount = fearCount;
    emit fearChanged(_fearCount);
    emit dirty();
}

bool Campaign::validateCampaignIds()
{
    QList<QUuid> knownIds;

    _isValid = validateSingleId(knownIds, this);

    qDebug() << "[Campaign] IDs validated result = " << _isValid << ",  " << knownIds.count() << " unique IDs";

    return _isValid;
}

bool Campaign::correctDuplicateIds()
{
    QList<QUuid> knownIds;

    bool validCampaign = validateSingleId(knownIds, this, true);

    qDebug() << "[Campaign] Duplicate IDs corrected: result = " << validCampaign << ",  " << knownIds.count() << " unique IDs";

    return validCampaign;
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
    if(_fearCount > 0)
        element.setAttribute("fear", _fearCount);

    if(_notes.count() > 0)
    {
        QDomElement notesElement = doc.createElement("notes");
        QDomCDATASection notesData = doc.createCDATASection(_notes.join(QString("\n")));
        notesElement.appendChild(notesData);
        element.appendChild(notesElement);
    }

    _ruleset.outputXML(doc, element, targetDirectory, isExport);

    QDomElement soundboardElement = doc.createElement("soundboard");
    for(SoundboardGroup* group : _soundboardGroups)
    {
        if(group)
        {
            QDomElement groupElement = doc.createElement("soundboardgroup");
            group->outputXML(doc, groupElement, targetDirectory, isExport);
            soundboardElement.appendChild(groupElement);
        }
    }
    element.appendChild(soundboardElement);

    if(!_overlays.isEmpty())
    {
        QDomElement overlaysElement = doc.createElement("overlays");
        for(Overlay* overlay : _overlays)
        {
            if(overlay)
            {
                QDomElement overlayElement = overlay->outputXML(doc, overlaysElement, targetDirectory);
                if(!overlayElement.isNull())
                    overlaysElement.appendChild(overlayElement);
            }
        }
        element.appendChild(overlaysElement);
    }
}

bool Campaign::belongsToObject(QDomElement& element)
{
    if((element.tagName() == QString("soundboard")) ||
       (element.tagName() == QString("ruleset")) ||
       (element.tagName() == QString("notes")) ||
       (element.tagName() == QString("overlays")))
        return true;
    else
        return CampaignObjectBase::belongsToObject(element);
}

void Campaign::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    QDomElement soundboardElement = element.firstChildElement("soundboard");
    if(!soundboardElement.isNull())
    {
        QDomElement groupElement = soundboardElement.firstChildElement("soundboardgroup");
        while(!groupElement.isNull())
        {
            SoundboardGroup* group = new SoundboardGroup(*this, groupElement, isImport);
            addSoundboardGroup(group);
            groupElement = groupElement.nextSiblingElement("soundboardgroup");
        }
    }

    CampaignObjectBase::internalPostProcessXML(element, isImport);
}

void Campaign::loadOverlayXML(const QDomElement &element)
{
    QDomElement overlayElement = element.firstChildElement(QString("overlay"));
    while(!overlayElement.isNull())
    {
        int overlayType = overlayElement.attribute(QString("type"), QString("-1")).toInt();
        Overlay* overlay = nullptr;
        switch(overlayType)
        {
        case DMHelper::OverlayType_Fear:
            overlay = new OverlayFear();
            break;
        case DMHelper::OverlayType_Counter:
            overlay = new OverlayCounter();
            break;
        case DMHelper::OverlayType_Timer:
            overlay = new OverlayTimer();
            break;
        default:
            break;
        }

        if(overlay)
        {
            overlay->inputXML(overlayElement);
            addOverlay(overlay);
        }

        overlayElement = overlayElement.nextSiblingElement(QString("overlay"));
    }
}

bool Campaign::validateSingleId(QList<QUuid>& knownIds, CampaignObjectBase* baseObject, bool correctDuplicates)
{
    if(!baseObject)
        return false;

    bool result = false;
    if(knownIds.contains(baseObject->getID()))
    {
        qCritical() << "[Campaign] duplicated campaign id: " << baseObject->getID() << " with name " << baseObject->getName();
        if(correctDuplicates)
            baseObject->renewID();
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
        if(!validateSingleId(knownIds, childList.at(i), correctDuplicates))
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
