#include "undomarker.h"
#include "mapframe.h"
#include "mapmarkergraphicsitem.h"
#include "map.h"
#include "dmconstants.h"
#include <QDomElement>

UndoMarker::UndoMarker(Map& map, const MapMarker& marker) :
    UndoBase(map, QString("Set Marker")),
    _marker(marker),
    _markerGraphicsItem(nullptr)
{
}

UndoMarker::~UndoMarker()
{
//    delete _markerGraphicsItem;
}

void UndoMarker::undo()
{
    delete _markerGraphicsItem;
    _markerGraphicsItem = nullptr;
}

void UndoMarker::redo()
{
    //delete _markerGraphicsItem;
    //_markerGraphicsItem = _map.addMapMarker(this, &_marker);
    if(_markerGraphicsItem)
        undo();

    _map.addMapMarker(this, &_marker);
}

void UndoMarker::apply(bool preview, QPaintDevice* target) const
{
    Q_UNUSED(preview);
    Q_UNUSED(target);
}

QDomElement UndoMarker::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("x", _marker.position().x());
    element.setAttribute("y", _marker.position().y());
    element.setAttribute("title", _marker.title());
    element.setAttribute("description", _marker.description());
    element.setAttribute("encounter", _marker.encounter().toString());

    return element;
}

void UndoMarker::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _marker.setX(element.attribute(QString("x")).toInt());
    _marker.setY(element.attribute(QString("y")).toInt());
    _marker.setTitle(element.attribute( QString("title") ));
    _marker.setDescription(element.attribute( QString("description") ));
    _marker.setEncounter(QUuid(element.attribute(QString("encounter"))));
}

int UndoMarker::getType() const
{
    return DMHelper::ActionType_SetMarker;
}

UndoBase* UndoMarker::clone() const
{
    return new UndoMarker(_map, _marker);
}

void UndoMarker::setTitle(const QString& title)
{
    _marker.setTitle(title);
    if(_markerGraphicsItem)
        _markerGraphicsItem->setTitle(title);
}

void UndoMarker::setDescription(const QString& description)
{
    _marker.setDescription(description);
    if(_markerGraphicsItem)
        _markerGraphicsItem->setDescription(description);
}

const MapMarker& UndoMarker::marker() const
{
    return _marker;
}

MapMarker& UndoMarker::marker()
{
    return _marker;
}

MapMarkerGraphicsItem* UndoMarker::getMarkerItem() const
{
    return _markerGraphicsItem;
}

void UndoMarker::setMarkerItem(MapMarkerGraphicsItem* markerItem)
{
    if(!markerItem)
        return;

    delete _markerGraphicsItem;
    _markerGraphicsItem = markerItem;
}

