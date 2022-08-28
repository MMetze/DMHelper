#include "undomarker.h"
#include "mapframe.h"
#include "mapmarkergraphicsitem.h"
#include "map.h"
#include "dmconstants.h"
#include <QDomElement>

UndoMarker::UndoMarker(Map* map, const MapMarker& marker) :
    UndoFowBase(map, QString("Set Marker")),
    _marker(marker),
    _markerGraphicsItem(nullptr)
{
}

UndoMarker::~UndoMarker()
{
}

void UndoMarker::undo()
{
    delete _markerGraphicsItem;
    _markerGraphicsItem = nullptr;
}

void UndoMarker::redo()
{
    if(_map)
    {
        if(_markerGraphicsItem)
            undo();

        _map->addMapMarker(this, &_marker);
    }
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

    element.setAttribute("x", _marker.getPosition().x());
    element.setAttribute("y", _marker.getPosition().y());
    element.setAttribute("playerVisible", _marker.isPlayerVisible());
    element.setAttribute("title", _marker.getTitle());
    element.setAttribute("description", _marker.getDescription());
    element.setAttribute("color", _marker.getColor().name());
    element.setAttribute("icon", _marker.getIconFile());
    element.setAttribute("iconScale", _marker.getIconScale());
    element.setAttribute("coloredIcon", _marker.isColoredIcon());
    element.setAttribute("encounter", _marker.getEncounter().toString());

    return element;
}

void UndoMarker::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _marker.setX(element.attribute(QString("x")).toInt());
    _marker.setY(element.attribute(QString("y")).toInt());
    _marker.setPlayerVisible(static_cast<bool>(element.attribute("playerVisible", QString::number(0)).toInt()));
    _marker.setTitle(element.attribute(QString("title")));
    _marker.setDescription(element.attribute(QString("description")));
    _marker.setIconFile(element.attribute("icon", QString(":/img/data/icon_pin.png")));
    _marker.setIconScale(element.attribute("iconScale", QString::number(25)).toInt());
    _marker.setColoredIcon(static_cast<bool>(element.attribute("coloredIcon", QString::number(0)).toInt()));
    _marker.setEncounter(QUuid(element.attribute(QString("encounter"))));

    QString colorName = element.attribute("color");
    _marker.setColor(QColor::isValidColor(colorName) ? QColor(colorName) : QColor(115,18,0));
}

void UndoMarker::setRemoved(bool removed)
{
    if(isRemoved() != removed)
    {
        UndoFowBase::setRemoved(removed);
        if(_markerGraphicsItem)
            _markerGraphicsItem->setVisible(removed);
    }
}

int UndoMarker::getType() const
{
    return DMHelper::ActionType_SetMarker;
}

UndoFowBase* UndoMarker::clone() const
{
    return new UndoMarker(_map, _marker);
}

void UndoMarker::setMarker(const MapMarker& marker)
{
    _marker = marker;
    if(_markerGraphicsItem)
        _markerGraphicsItem->setMarker(marker);
}

const MapMarker& UndoMarker::getMarker() const
{
    return _marker;
}

MapMarker& UndoMarker::getMarker()
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
    _markerGraphicsItem->setVisible(!isRemoved());
}
