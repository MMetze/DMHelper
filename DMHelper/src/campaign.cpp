#include "campaign.h"
#include "adventure.h"
#include "character.h"
#include "encounter.h"
#include "encounterfactory.h"
#include "map.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include "basicdateserver.h"
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QDebug>

Campaign::Campaign(const QString& campaignName, QObject *parent) :
    CampaignObjectBase(parent),
    _name(campaignName),
    _notes(nullptr),
    characters(),
    adventures(),
    settings(),
    npcs(),
    tracks(),
    _partyExpanded(false),
    _adventuresExpanded(false),
    _worldExpanded(false),
    _worldSettingsExpanded(false),
    _worldNPCsExpanded(false),
    _date(1,1,0),
    _time(0,0),
    _batchChanges(false),
    _changesMade(false),
    _dirtyMade(false),
    _isValid(true)
{
    //setID(1);
    //CampaignObjectBase::resetBaseId();

    _notes = EncounterFactory::createEncounter(DMHelper::EncounterType_Text, QString(""), this);
    connect(_notes,SIGNAL(dirty()),this,SLOT(handleInternalDirty()));
}

Campaign::Campaign(const QDomElement& element, bool isImport, QObject *parent) :
    CampaignObjectBase(parent),
    _name(),
    _notes(nullptr),
    characters(),
    adventures(),
    settings(),
    npcs(),
    tracks(),
    _partyExpanded(false),
    _adventuresExpanded(false),
    _worldExpanded(false),
    _worldSettingsExpanded(false),
    _worldNPCsExpanded(false),
    _date(1,1,0),
    _time(0,0),
    _batchChanges(false),
    _changesMade(false),
    _dirtyMade(false),
    _isValid(false)
{
    inputXML(element, isImport);
    postProcessXML(element, isImport);
}

Campaign::~Campaign()
{
    cleanupCampaign(true);
}

void Campaign::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement campaignElement = doc.createElement( "campaign" );

    CampaignObjectBase::outputXML(doc, campaignElement, targetDirectory, isExport);

    campaignElement.setAttribute( "name", getName() );
    campaignElement.setAttribute( "majorVersion", DMHelper::CAMPAIGN_MAJOR_VERSION );
    campaignElement.setAttribute( "minorVersion", DMHelper::CAMPAIGN_MINOR_VERSION );
    campaignElement.setAttribute( "partyExpanded", static_cast<int>(getPartyExpanded()) );
    campaignElement.setAttribute( "adventuresExpanded", static_cast<int>(getAdventuresExpanded()) );
    campaignElement.setAttribute( "worldExpanded", static_cast<int>(getWorldExpanded()) );
    campaignElement.setAttribute( "worldSettingsExpanded", static_cast<int>(getWorldSettingsExpanded()) );
    campaignElement.setAttribute( "worldNPCsExpanded", static_cast<int>(getWorldNPCsExpanded()) );
    campaignElement.setAttribute( "calendar", BasicDateServer::Instance() ? BasicDateServer::Instance()->getActiveCalendarName() : QString() );
    campaignElement.setAttribute( "date", getDate().toStringDDMMYYYY() );
    campaignElement.setAttribute( "time", getTime().msecsSinceStartOfDay() );
    parent.appendChild(campaignElement);

    QDomElement notesElement = doc.createElement( "notes" );
    campaignElement.appendChild(notesElement);
    if(_notes)
        _notes->outputXML(doc, notesElement, targetDirectory, isExport);

    QDomElement charactersElement = doc.createElement( "characters" );
    campaignElement.appendChild(charactersElement);
    for( int charIt = 0; charIt < characters.size(); ++charIt )
    {
        characters.at(charIt)->outputXML(doc, charactersElement, targetDirectory, isExport);
    }

    QDomElement adventuresElement = doc.createElement( "adventures" );
    campaignElement.appendChild(adventuresElement);
    for( int advIt = 0; advIt < adventures.size(); ++advIt )
    {
        adventures.at(advIt)->outputXML(doc, adventuresElement, targetDirectory, isExport);
    }

    QDomElement settingsElement = doc.createElement( "settings" );
    campaignElement.appendChild(settingsElement);
    for( int settingIt = 0; settingIt < settings.size(); ++settingIt )
    {
        settings.at(settingIt)->outputXML(doc, settingsElement, targetDirectory, isExport);
    }

    QDomElement npcsElement = doc.createElement( "npcs" );
    campaignElement.appendChild(npcsElement);
    for( int npcIt = 0; npcIt < npcs.size(); ++npcIt )
    {
        npcs.at(npcIt)->outputXML(doc, npcsElement, targetDirectory, isExport);
    }

    QDomElement tracksElement = doc.createElement( "tracks" );
    campaignElement.appendChild(tracksElement);
    for( int trackIt = 0; trackIt < tracks.size(); ++trackIt )
    {
        tracks.at(trackIt)->outputXML(doc, tracksElement, targetDirectory, isExport);
    }
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

    CampaignObjectBase::inputXML(element, isImport);

    int encounterCount = 0;
    int mapCount = 0;

    _name = element.attribute("name");
    _partyExpanded = static_cast<bool>(element.attribute("partyExpanded",QString::number(0)).toInt());
    _adventuresExpanded = static_cast<bool>(element.attribute("adventuresExpanded",QString::number(0)).toInt());
    _worldExpanded = static_cast<bool>(element.attribute("worldExpanded",QString::number(0)).toInt());
    _worldSettingsExpanded = static_cast<bool>(element.attribute("worldSettingsExpanded",QString::number(0)).toInt());
    _worldNPCsExpanded = static_cast<bool>(element.attribute("worldNPCsExpanded",QString::number(0)).toInt());
    QString calendarName = element.attribute("calendar", QString("Gregorian"));
    if(BasicDateServer::Instance())
        BasicDateServer::Instance()->setActiveCalendar(calendarName);
    BasicDate inputDate(element.attribute("date",QString("")));
    setDate(inputDate);
    setTime(QTime::fromMSecsSinceStartOfDay(element.attribute("time",QString::number(0)).toInt()));

    QDomElement notesElement = element.firstChildElement( QString("notes") );
    _notes = EncounterFactory::createEncounter(DMHelper::EncounterType_Text, notesElement.firstChildElement(), isImport, this);
    connect(_notes,SIGNAL(dirty()),this,SLOT(handleInternalDirty()));

    QDomElement charactersElement = element.firstChildElement( QString("characters") );
    if( !charactersElement.isNull() )
    {
        QDomElement characterElement = charactersElement.firstChildElement( QString("combatant") );
        while( !characterElement.isNull() )
        {
            Character* newCharacter = new Character(characterElement, isImport);
            addCharacter(newCharacter);
            characterElement = characterElement.nextSiblingElement( QString("combatant") );
        }
    }

    QDomElement adventuresElement = element.firstChildElement( QString("adventures") );
    if( !adventuresElement.isNull() )
    {
        QDomElement adventureElement = adventuresElement.firstChildElement( QString("adventure") );
        while( !adventureElement.isNull() )
        {
            Adventure* newAdventure = new Adventure(adventureElement, isImport);
            addAdventure(newAdventure);

            encounterCount += newAdventure->getEncounterCount();
            mapCount += newAdventure->getMapCount();

            adventureElement = adventureElement.nextSiblingElement( QString("adventure") );
        }
    }

    QDomElement settingsElement = element.firstChildElement( QString("settings") );
    if( !settingsElement.isNull() )
    {
        QDomElement mapElement = settingsElement.firstChildElement( QString("map") );
        while( !mapElement.isNull() )
        {
            Map* newMap = new Map(mapElement, isImport);
            addSetting(newMap);
            mapElement = mapElement.nextSiblingElement( QString("map") );
        }
    }

    QDomElement npcsElement = element.firstChildElement( QString("npcs") );
    if( !npcsElement.isNull() )
    {
        QDomElement characterElement = npcsElement.firstChildElement( QString("combatant") );
        while( !characterElement.isNull() )
        {
            Character* newCharacter = new Character(characterElement, isImport);
            addNPC(newCharacter);
            characterElement = characterElement.nextSiblingElement( QString("combatant") );
        }
    }

    QDomElement tracksElement = element.firstChildElement( QString("tracks") );
    if( !tracksElement.isNull() )
    {
        QDomElement trackElement = tracksElement.firstChildElement( QString("track") );
        while( !trackElement.isNull() )
        {
            AudioTrack* newTrack = new AudioTrack(trackElement, isImport);
            addTrack(newTrack);
            trackElement = trackElement.nextSiblingElement( QString("track") );
        }
    }

    // Sum up all the elements loaded. The +2 is for the campaign object itself and the notes object
    int totalElements = characters.count() + settings.count() + npcs.count() + adventures.count() + tracks.count() + encounterCount + mapCount + 2;

    qDebug() << "[Campaign] Loaded campaign """ << _name << """ containing " << totalElements << " elements";
    qDebug() << "           Date: " << _date.toStringDDMMYYYY() << ", Time: " << _time;
    qDebug() << "           Party: " << characters.count() << " characters";
    qDebug() << "           Settings: " << settings.count();
    qDebug() << "           NPCs: " << npcs.count();
    qDebug() << "           Adventures: " << adventures.count();
    qDebug() << "               Encounters: " << encounterCount;
    qDebug() << "               Maps: " << mapCount;
    qDebug() << "           Audio Tracks: " << tracks.count();

    validateCampaignIds();
}

void Campaign::postProcessXML(const QDomElement &element, bool isImport)
{
    QDomElement charactersElement = element.firstChildElement( QString("characters") );
    if( !charactersElement.isNull() )
    {
        QDomElement characterElement = charactersElement.firstChildElement( QString("combatant") );
        while( !characterElement.isNull() )
        {
            Character* character = getCharacterById(parseIdString(characterElement.attribute( QString("_baseID") )));
            if(character)
                character->postProcessXML(characterElement, isImport);
            characterElement = characterElement.nextSiblingElement( QString("combatant") );
        }
    }

    QDomElement adventuresElement = element.firstChildElement( QString("adventures") );
    if( !adventuresElement.isNull() )
    {
        QDomElement adventureElement = adventuresElement.firstChildElement( QString("adventure") );
        while( !adventureElement.isNull() )
        {
            Adventure* adventure = getAdventureById(parseIdString(adventureElement.attribute( QString("_baseID") )));
            if(adventure)
                adventure->postProcessXML(adventureElement, isImport);
            adventureElement = adventureElement.nextSiblingElement( QString("adventure") );
        }
    }

    QDomElement settingsElement = element.firstChildElement( QString("settings") );
    if( !settingsElement.isNull() )
    {
        QDomElement mapElement = settingsElement.firstChildElement( QString("map") );
        while( !mapElement.isNull() )
        {
            Map* map = getSettingById(parseIdString(mapElement.attribute( QString("_baseID") )));
            if(map)
                map->postProcessXML(mapElement, isImport);
            mapElement = mapElement.nextSiblingElement( QString("map") );
        }
    }

    QDomElement npcsElement = element.firstChildElement( QString("npcs") );
    if( !npcsElement.isNull() )
    {
        QDomElement characterElement = npcsElement.firstChildElement( QString("combatant") );
        while( !characterElement.isNull() )
        {
            Character* character = getNPCById(parseIdString(characterElement.attribute( QString("_baseID") )));
            if(character)
                character->postProcessXML(characterElement, isImport);
            characterElement = characterElement.nextSiblingElement( QString("combatant") );
        }
    }

    CampaignObjectBase::postProcessXML(element, isImport);
}

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


void Campaign::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
    _dirtyMade = false;
}

void Campaign::endBatchChanges()
{
    _batchChanges = false;
    _changesMade = false;
    if(_changesMade)
        emit changed();
    if(_dirtyMade)
        emit dirty();
}

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

Encounter* Campaign::getNotes() const
{
    return _notes;
}

int Campaign::getCharacterCount()
{
    return characters.count();
}

Character* Campaign::getCharacterById(QUuid id)
{
    for(int i = 0; i < characters.count(); ++i)
    {
        if(characters.at(i)->getID() == id)
            return characters.at(i);
    }

    return nullptr;
}

const Character* Campaign::getCharacterById(QUuid id) const
{
    for(int i = 0; i < characters.count(); ++i)
    {
        if(characters.at(i)->getID() == id)
            return characters.at(i);
    }

    return nullptr;
}

Character* Campaign::getCharacterByDndBeyondId(int id)
{
    for(int i = 0; i < characters.count(); ++i)
    {
        if(characters.at(i)->getDndBeyondID() == id)
            return characters.at(i);
    }

    return nullptr;
}

Character* Campaign::getCharacterOrNPCByDndBeyondId(int id)
{
    for(int i = 0; i < characters.count(); ++i)
    {
        if(characters.at(i)->getDndBeyondID() == id)
            return characters.at(i);
    }

    for(int j = 0; j < npcs.count(); ++j)
    {
        if(npcs.at(j)->getDndBeyondID() == id)
            return npcs.at(j);
    }

    return nullptr;
}

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

QList<Character*> Campaign::getActiveCharacters()
{
    QList<Character*> actives;

    for(int i = 0; i < characters.count(); ++i)
    {
        if(characters.at(i)->getActive())
            actives.append(characters.at(i));
    }

    return actives;
}

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

Adventure* Campaign::getAdventureById(QUuid id)
{
    for(int i = 0; i < adventures.count(); ++i)
    {
        if(adventures.at(i)->getID() == id)
            return adventures.at(i);
    }

    return nullptr;
}

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

Character* Campaign::getNPCById(QUuid id)
{
    for(int i = 0; i < npcs.count(); ++i)
    {
        if(npcs.at(i)->getID() == id)
            return npcs.at(i);
    }

    return nullptr;
}

const Character* Campaign::getNPCById(QUuid id) const
{
    for(int i = 0; i < npcs.count(); ++i)
    {
        if(npcs.at(i)->getID() == id)
            return npcs.at(i);
    }

    return nullptr;
}

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

AudioTrack* Campaign::getTrackById(QUuid id)
{
    for(int i = 0; i < tracks.count(); ++i)
    {
        if(tracks.at(i)->getID() == id)
            return tracks.at(i);
    }

    return nullptr;
}

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

    delete _notes;
    _notes = nullptr;

    if(deleteAll)
    {
        qDeleteAll(characters);
        qDeleteAll(adventures);
        qDeleteAll(settings);
        qDeleteAll(npcs);
        qDeleteAll(tracks);
    }
    else
    {
        characters.clear();
        adventures.clear();
        settings.clear();
        npcs.clear();
        tracks.clear();
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
    bool validResult;
    QList<QUuid> knownIds;

    validResult = validateSingleId(knownIds, this);
    validResult = validResult && validateSingleId(knownIds, _notes);

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

    qDebug() << "[Campaign] IDs validated result = " << validResult << ",  " << knownIds.count()<< " unique IDs";

    _isValid = validResult;
    return _isValid;
}

void Campaign::handleInteralChange()
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

bool Campaign::validateSingleId(QList<QUuid>& knownIds, CampaignObjectBase* baseObject)
{
    if(!baseObject)
        return false;

    if(knownIds.contains(baseObject->getID()))
    {
        qCritical() << "[Campaign] duplicated campaign id: " << baseObject->getID();
        return false;
    }
    else
    {
        knownIds.append(baseObject->getID());
        qSort(knownIds.begin(), knownIds.end());
        return true;
    }
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
