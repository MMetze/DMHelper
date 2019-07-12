#include "mapmarkergraphicsitem.h"
#include "mapmarkerdialog.h"
#include "mapframe.h"
#include <QGraphicsScene>

const int MARKER_SIZE = 20;

MapMarkerGraphicsItem::MapMarkerGraphicsItem(QGraphicsScene* scene, const MapMarker& marker, MapFrame& mapFrame) :
    QGraphicsItemGroup(),
    _marker(marker.getID()),
    _mapFrame(mapFrame),
    _markerIcon(nullptr),
    _title(nullptr),
    _details(nullptr),
    _detailsVisible(false),
    _clicked(false)
{
    _markerIcon = scene->addPixmap(QPixmap(":/img/data/map_marker.png").scaled(MARKER_SIZE, MARKER_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    _markerIcon->setFlag(QGraphicsItem::ItemIsMovable);
    _markerIcon->setPos(-MARKER_SIZE / 2, -MARKER_SIZE / 2);
    addToGroup(_markerIcon);

    setFlag(QGraphicsItem::ItemIsMovable);
    scene->addItem(this);
}

void MapMarkerGraphicsItem::setGroupVisible(bool visible)
{
    if(visible)
    {
        _markerIcon->setVisible(true);
        _title->setVisible(true);
        _details->setVisible(_detailsVisible);
    }
    else
    {
        _markerIcon->setVisible(false);
        _title->setVisible(false);
        _details->setVisible(false);
    }
}

QVariant MapMarkerGraphicsItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if((change == QGraphicsItem::ItemSelectedChange) && (value.toBool() == false))
    {
        // Deselected
        _details->setVisible(false);
    }
    else if(change == QGraphicsItem::ItemPositionHasChanged)
    {
        _mapFrame.mapMarkerMoved(_marker);
    }

    return QGraphicsItem::itemChange(change, value);
}

void MapMarkerGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    _mapFrame.editMapMarker(_marker);

    QGraphicsItem::mouseDoubleClickEvent(event);
}

void MapMarkerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    _clicked = false;

    QGraphicsItem::mouseMoveEvent(event);
}

void MapMarkerGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    _clicked = true;

    QGraphicsItem::mousePressEvent(event);
}

void MapMarkerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if(_clicked)
    {
        toggleDetails();
        _clicked = false;
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void MapMarkerGraphicsItem::toggleDetails()
{
    _detailsVisible = !_detailsVisible;
    _details->setVisible(_detailsVisible);
}

int MapMarkerGraphicsItem::type() const
{
    return Type;
}
