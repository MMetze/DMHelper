#include "adventure.h"
#include "encounter.h"
#include "encountertext.h"
#include "encounterfactory.h"
#include "map.h"
#include <QDomDocument>
#include <QDomElement>

Adventure::Adventure(const QString& adventureName, QObject *parent) :
    CampaignObjectBase(parent),
    _name(adventureName),
    encounters(),
    maps(),
    _expanded(false)
{
}

Adventure::Adventure(const QDomElement& element, QObject *parent) :
    CampaignObjectBase(parent),
    _name(),
    encounters(),
    maps(),
    _expanded(false)
{
    inputXML(element);
}

Adventure::~Adventure()
{
    qDeleteAll(encounters);
    qDeleteAll(maps);
}

int Adventure::getEncounterCount()
{
    return encounters.count();
}

Encounter* Adventure::getEncounterById(int id)
{
    for(int i = 0; i < encounters.count(); ++i)
    {
        if(encounters.at(i)->getID() == id)
            return encounters.at(i);
    }

    return NULL;
}

Encounter* Adventure::getEncounterByIndex(int index)
{
    if((index < 0)||(index >= encounters.size()))
        return NULL;

    return encounters.at(index);
}

int Adventure::addEncounter(Encounter* newItem)
{
    return addEncounter(newItem, encounters.count());
}

int Adventure::addEncounter(Encounter* newItem, int index)
{
    if(!newItem)
        return DMH_GLOBAL_INVALID_ID;

    if(index < 0)
        index = 0;
    if(index > encounters.count())
        index = encounters.count();

    newItem->setParent(this);
    connect(newItem,SIGNAL(dirty()),this,SIGNAL(dirty()));
    connect(newItem,SIGNAL(changed()),this,SIGNAL(changed()));
    encounters.insert(index, newItem);
    emit changed();
    emit dirty();
    return newItem->getID();
}

Encounter* Adventure::removeEncounter(int id)
{
    Encounter* encounter = getEncounterById(id);
    if(encounter)
    {
        if(encounters.removeOne(encounter))
        {
            emit changed();
            emit dirty();
            return encounter;
        }
    }

    return NULL;
}

void Adventure::moveEncounterTo(int id, int index)
{
    if(index < 0)
        index = 0;
    if(index >= encounters.count())
        index = encounters.count() - 1;

    for(int i = 0; i < encounters.count(); ++i)
    {
        if(encounters.at(i)->getID() == id)
        {
            encounters.move(i, index > i ? index - 1 : index);
            emit changed();
            emit dirty();
            return;
        }
    }
}

int Adventure::getMapCount()
{
    return maps.count();
}

Map* Adventure::getMapById(int id)
{
    for(int i = 0; i < maps.count(); ++i)
    {
        if(maps.at(i)->getID() == id)
            return maps.at(i);
    }

    return NULL;
}

Map* Adventure::getMapByIndex(int index)
{
    if((index < 0)||(index >= maps.size()))
        return NULL;

    return maps.at(index);
}

int Adventure::addMap(Map* newItem)
{
    return addMap(newItem, maps.count());
}

int Adventure::addMap(Map* newItem, int index)
{
    if(!newItem)
        return DMH_GLOBAL_INVALID_ID;

    if(index < 0)
        index = 0;
    if(index > maps.count())
        index = maps.count();

    newItem->setParent(this);
    connect(newItem,SIGNAL(dirty()),this,SIGNAL(dirty()));
    connect(newItem,SIGNAL(changed()),this,SIGNAL(changed()));
    maps.insert(index, newItem);
    emit changed();
    emit dirty();
    return newItem->getID();
}

Map* Adventure::removeMap(int id)
{
    Map* map = getMapById(id);
    if(map)
    {
        if(maps.removeOne(map))
        {
            emit changed();
            emit dirty();
            return map;
        }
    }

    return NULL;
}

void Adventure::moveMapTo(int id, int index)
{
    if(index < 0)
        index = 0;
    if(index >= maps.count())
        index = maps.count() - 1;

    for(int i = 0; i < maps.count(); ++i)
    {
        if(maps.at(i)->getID() == id)
        {
            maps.move(i, index > i ? index - 1 : index);
            emit changed();
            emit dirty();
            return;
        }
    }
}

void Adventure::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    QDomElement adventureElement = doc.createElement( "adventure" );

    CampaignObjectBase::outputXML(doc, adventureElement, targetDirectory);

    adventureElement.setAttribute( "name", getName() );
    adventureElement.setAttribute( "expanded", (int)getExpanded() );
    parent.appendChild(adventureElement);

    QDomElement encountersElement = doc.createElement( "encounters" );
    adventureElement.appendChild(encountersElement);
    for( int encIt = 0; encIt < encounters.size(); ++encIt )
    {
        encounters.at(encIt)->outputXML(doc, encountersElement, targetDirectory);
    }

    QDomElement mapsElement = doc.createElement( "maps" );
    adventureElement.appendChild(mapsElement);
    for( int mapIt = 0; mapIt < maps.size(); ++mapIt )
    {
        maps.at(mapIt)->outputXML(doc, mapsElement, targetDirectory);
    }
}

void Adventure::inputXML(const QDomElement &element)
{
    CampaignObjectBase::inputXML(element);

    _name = element.attribute("name");
    _expanded = (bool)(element.attribute("expanded",QString::number(0)).toInt());

    QDomElement encountersElement = element.firstChildElement( QString("encounters") );
    if( !encountersElement.isNull() )
    {
        QDomElement encounterElement = encountersElement.firstChildElement( QString("encounter") );
        while( !encounterElement.isNull() )
        {
            bool ok = false;
            int encounterType = encounterElement.attribute("type").toInt(&ok);
            if(ok)
            {
                Encounter* newEncounter = EncounterFactory::createEncounter(encounterType, encounterElement, this);
                if(newEncounter)
                {
                    addEncounter(newEncounter);
                }
            }
            encounterElement = encounterElement.nextSiblingElement( QString("encounter") );
        }
    }

    QDomElement mapsElement = element.firstChildElement( QString("maps") );
    if( !mapsElement.isNull() )
    {
        QDomElement mapElement = mapsElement.firstChildElement( QString("map") );
        while( !mapElement.isNull() )
        {
            Map* newMap = new Map(mapElement);
            addMap(newMap);
            mapElement = mapElement.nextSiblingElement( QString("map") );
        }
    }
}

void Adventure::postProcessXML(const QDomElement &element)
{
    QDomElement encountersElement = element.firstChildElement( QString("encounters") );
    if( !encountersElement.isNull() )
    {
        QDomElement encounterElement = encountersElement.firstChildElement( QString("encounter") );
        while( !encounterElement.isNull() )
        {
            Encounter* encounter = getEncounterById(encounterElement.attribute( QString("_baseID") ).toInt());
            if(encounter)
                encounter->postProcessXML(encounterElement);
            encounterElement = encounterElement.nextSiblingElement( QString("encounter") );
        }
    }

    QDomElement mapsElement = element.firstChildElement( QString("maps") );
    if( !mapsElement.isNull() )
    {
        QDomElement mapElement = mapsElement.firstChildElement( QString("map") );
        while( !mapElement.isNull() )
        {
            Map* map = getMapById(mapElement.attribute( QString("_baseID") ).toInt());
            if(map)
                map->postProcessXML(mapElement);
            mapElement = mapElement.nextSiblingElement( QString("map") );
        }
    }
}

QString Adventure::getName() const
{
    return _name;
}

void Adventure::setName(const QString& adventureName)
{
    if(_name != adventureName)
    {
        _name = adventureName;
        emit changed();
    }
}

bool Adventure::getExpanded() const
{
    return _expanded;
}

void Adventure::setExpanded(bool expanded)
{
    if(_expanded != expanded)
    {
        _expanded = expanded;
        emit dirty();
    }
}
