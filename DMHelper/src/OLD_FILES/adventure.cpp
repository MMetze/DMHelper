#include "adventure.h"
#include "encounter.h"
#include "encountertext.h"
#include "encounterfactory.h"
#include "map.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

Adventure::Adventure(const QString& adventureName, QObject *parent) :
    CampaignObjectBase(parent),
    _name(adventureName),
    encounters(),
    maps(),
    _expanded(false)
{
}

Adventure::Adventure(const QDomElement& element, bool isImport, QObject *parent) :
    CampaignObjectBase(parent),
    _name(),
    encounters(),
    maps(),
    _expanded(false)
{
    inputXML(element, isImport);
}

Adventure::Adventure(const Adventure &obj) :
    CampaignObjectBase(obj),
    _name(obj._name),
    encounters(),
    maps(),
    _expanded(obj._expanded)
{
    for(Encounter* encounter: obj.encounters)
    {
        addEncounter(EncounterFactory::cloneEncounter(*encounter));
    }

    for(Map* map : obj.maps)
    {
        addMap(new Map(*map));
    }
}

Adventure::~Adventure()
{
    qDeleteAll(encounters);
    qDeleteAll(maps);
}

Adventure* Adventure::createShellClone()
{
    qDebug() << "[Adventure]: WARNING - Creating a clone ID of the adventure " << _name << "! Only do this for very good reasons!!";

    Adventure* shell = new Adventure(_name);
    shell->setID(getID());
    return shell;
}

int Adventure::getEncounterCount()
{
    return encounters.count();
}

Encounter* Adventure::getEncounterById(QUuid id)
{
    for(int i = 0; i < encounters.count(); ++i)
    {
        if(encounters.at(i)->getID() == id)
            return encounters.at(i);
    }

    return nullptr;
}

Encounter* Adventure::getEncounterByIndex(int index)
{
    if((index < 0)||(index >= encounters.size()))
        return nullptr;

    return encounters.at(index);
}

QUuid Adventure::addEncounter(Encounter* newItem)
{
    return addEncounter(newItem, encounters.count());
}

QUuid Adventure::addEncounter(Encounter* newItem, int index)
{
    if(!newItem)
        return QUuid();

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

Encounter* Adventure::removeEncounter(QUuid id)
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

    return nullptr;
}

void Adventure::moveEncounterTo(QUuid id, int index)
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

Map* Adventure::getMapById(QUuid id)
{
    for(int i = 0; i < maps.count(); ++i)
    {
        if(maps.at(i)->getID() == id)
            return maps.at(i);
    }

    return nullptr;
}

Map* Adventure::getMapByIndex(int index)
{
    if((index < 0)||(index >= maps.size()))
        return nullptr;

    return maps.at(index);
}

QUuid Adventure::addMap(Map* newItem)
{
    return addMap(newItem, maps.count());
}

QUuid Adventure::addMap(Map* newItem, int index)
{
    if(!newItem)
        return QUuid();

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

Map* Adventure::removeMap(QUuid id)
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

    return nullptr;
}

void Adventure::moveMapTo(QUuid id, int index)
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

void Adventure::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement adventureElement = doc.createElement( "adventure" );

    CampaignObjectBase::outputXML(doc, adventureElement, targetDirectory, isExport);

    adventureElement.setAttribute( "name", getName() );
    adventureElement.setAttribute( "expanded", static_cast<int>(getExpanded()) );
    parent.appendChild(adventureElement);

    QDomElement encountersElement = doc.createElement( "encounters" );
    adventureElement.appendChild(encountersElement);
    for( int encIt = 0; encIt < encounters.size(); ++encIt )
    {.
        encounters.at(encIt)->outputXML(doc, encountersElement, targetDirectory, isExport);
    }

    QDomElement mapsElement = doc.createElement( "maps" );
    adventureElement.appendChild(mapsElement);
    for( int mapIt = 0; mapIt < maps.size(); ++mapIt )
    {
        maps.at(mapIt)->outputXML(doc, mapsElement, targetDirectory, isExport);
    }
}

void Adventure::inputXML(const QDomElement &element, bool isImport)
{
    CampaignObjectBase::inputXML(element, isImport);

    _name = element.attribute("name");
    _expanded = static_cast<bool>(element.attribute("expanded",QString::number(0)).toInt());

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
                Encounter* newEncounter = EncounterFactory::createEncounter(encounterType, encounterElement, isImport, this);
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
            Map* newMap = new Map(mapElement, isImport);
            addMap(newMap);
            mapElement = mapElement.nextSiblingElement( QString("map") );
        }
    }
}

void Adventure::postProcessXML(const QDomElement &element, bool isImport)
{
    QDomElement encountersElement = element.firstChildElement( QString("encounters") );
    if( !encountersElement.isNull() )
    {
        QDomElement encounterElement = encountersElement.firstChildElement( QString("encounter") );
        while( !encounterElement.isNull() )
        {
            Encounter* encounter = getEncounterById(parseIdString(encounterElement.attribute( QString("_baseID") )));
            if(encounter)
                encounter->postProcessXML(encounterElement, isImport);
            encounterElement = encounterElement.nextSiblingElement( QString("encounter") );
        }
    }

    QDomElement mapsElement = element.firstChildElement( QString("maps") );
    if( !mapsElement.isNull() )
    {
        QDomElement mapElement = mapsElement.firstChildElement( QString("map") );
        while( !mapElement.isNull() )
        {
            Map* map = getMapById(parseIdString(mapElement.attribute( QString("_baseID") )));
            if(map)
                map->postProcessXML(mapElement, isImport);
            mapElement = mapElement.nextSiblingElement( QString("map") );
        }
    }

    CampaignObjectBase::postProcessXML(element, isImport);
}

void Adventure::resolveReferences()
{
    for( int encIt = 0; encIt < encounters.size(); ++encIt )
    {
        encounters.at(encIt)->resolveReferences();
    }

    for( int mapIt = 0; mapIt < maps.size(); ++mapIt )
    {
        maps.at(mapIt)->resolveReferences();
    }
}

/*
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
*/

void Adventure::clear()
{
    encounters.clear();
    maps.clear();
}
