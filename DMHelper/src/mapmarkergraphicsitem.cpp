#include "mapmarkergraphicsitem.h"
#include "mapmarkerdialog.h"
#include "mapframe.h"
#include "scaledpixmap.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

const int MARKER_SIZE = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0];

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
    _markerIcon = new MapMarkerGraphicsPixmapItem(QPixmap(":/img/data/icon_mapmarker.png").scaled(MARKER_SIZE, MARKER_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    scene->addItem(_markerIcon);
    _markerIcon->setFlag(QGraphicsItem::ItemIsMovable);
    _markerIcon->setPos(-MARKER_SIZE / 2, -MARKER_SIZE);
    _markerIcon->setGroup(this);

    _title = new MapMarkerGraphicsSimpleTextItem(marker.title());
    scene->addItem(_title);
    _title->setBrush(QBrush(Qt::white));
    _title->setPen(QPen(QColor(115,18,0)));
    QFont titleFont = _title->font();
    titleFont.setPointSize(MARKER_SIZE / 8);
    _title->setFont(titleFont);
    _title->setGroup(this);

    _details = new MapMarkerGraphicsSimpleTextItem(marker.description());
    scene->addItem(_details);
    _details->setBrush(QBrush(Qt::white));
    _details->setPen(QPen(QColor(115,18,0)));
    _details->setVisible(_detailsVisible);
    QFont detailsFont = _details->font();
    detailsFont.setPointSize(MARKER_SIZE / 10);
    _details->setFont(detailsFont);
    _details->setPos(0, QFontMetrics(detailsFont).height() * 15 / 10);
    _details->setGroup(this);

    setFlag(QGraphicsItem::ItemIsMovable);
    scene->addItem(this);

    setCursor(Qt::PointingHandCursor);
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

void MapMarkerGraphicsItem::setTitle(const QString& title)
{
    if(_title)
        _title->setText(title);
}

void MapMarkerGraphicsItem::setDescription(const QString& description)
{
    if(_details)
        _details->setText(description);
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
        if((event) && ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
            _mapFrame.activateMapMarker(_marker);
        else
            toggleDetails();
        _clicked = false;
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void MapMarkerGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //if((event) && ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
    //    setCursor(Qt::PointingHandCursor);
    //else
    //    unsetCursor();
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
