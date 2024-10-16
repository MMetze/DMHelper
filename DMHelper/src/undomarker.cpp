#include "undomarker.h"
#include "layerfow.h"
#include "mapmarkergraphicsitem.h"
#include "dmconstants.h"
#include <QDomElement>

// TODO: separate marker layer

//UndoMarker::UndoMarker(LayerFow* layer, const MapMarker& marker) :
UndoMarker::UndoMarker(const MapMarker& marker) :
//    UndoFowBase(layer, QString("Set Marker")),
    QObject(),
    _marker(marker),
    _markerGraphicsItem(nullptr)//,
    //_removed(false)
{
}

UndoMarker::~UndoMarker()
{
}

/*
void UndoMarker::undo()
{
    delete _markerGraphicsItem;
    _markerGraphicsItem = nullptr;
}
*/

/*
void UndoMarker::redo()
{
    if(_map)
    {
        if(_markerGraphicsItem)
            undo();

        _map->addMapMarker(this, &_marker);
    }
}
*/
/*
void UndoMarker::apply()
{
}
*/

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
    _marker.setColor(QColor::isValidColor(colorName) ? QColor(colorName) : QColor(115, 18, 0));
}

/*
bool UndoMarker::isRemoved() const
{
    return _removed;
}

void UndoMarker::setRemoved(bool removed)
{
    if(isRemoved() != removed)
    {
        _removed = removed;
        if(_markerGraphicsItem)
            _markerGraphicsItem->setVisible(removed);
    }
}
*/
/*
int UndoMarker::getType() const
{
    return DMHelper::ActionType_SetMarker;
}
*/

UndoMarker* UndoMarker::clone() const
{
    return new UndoMarker(_marker);
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

void UndoMarker::createMarkerItem(QGraphicsScene* scene, qreal initialScale)
{
    cleanupMarkerItem();

    _markerGraphicsItem = new MapMarkerGraphicsItem(scene, this, initialScale);
    _markerGraphicsItem->setPos(_marker.getPosition());
    _markerGraphicsItem->setZValue(DMHelper::BattleDialog_Z_BackHighlight);
}

void UndoMarker::cleanupMarkerItem()
{
    if(_markerGraphicsItem)
    {
        delete _markerGraphicsItem;
        _markerGraphicsItem = nullptr;
    }
}

MapMarkerGraphicsItem* UndoMarker::getMarkerItem() const
{
    return _markerGraphicsItem;
}

/*
void UndoMarker::setMarkerItem(MapMarkerGraphicsItem* markerItem)
{
    if(!markerItem)
        return;

    delete _markerGraphicsItem;
    _markerGraphicsItem = markerItem;
    //_markerGraphicsItem->setVisible(!isRemoved());
    _markerGraphicsItem->setVisible(true);
}
*/

void UndoMarker::moveMapMarker()
{
    emit mapMarkerMoved(this);
}

void UndoMarker::editMapMarker()
{
    emit mapMarkerEdited(this);
}

void UndoMarker::unsetPartySelected()
{
    emit unselectParty(false);
}

void UndoMarker::activateMapMarker()
{
    emit mapMarkerActivated(this);
}
