#include "mapmarkergraphicsitem.h"
#include "mapframe.h"
#include "scaledpixmap.h"
#include "mapcolorizefilter.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

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
    _markerIcon = new MapMarkerGraphicsPixmapItem(this);
    scene->addItem(_markerIcon);
    _markerIcon->setFlag(QGraphicsItem::ItemIsMovable);
    _markerIcon->setGroup(this);

    _title = new MapMarkerGraphicsSimpleTextItem(this);
    scene->addItem(_title);
    _title->setPen(QPen(Qt::white));
    _title->setGroup(this);

    _details = new MapMarkerGraphicsSimpleTextItem(this);
    scene->addItem(_details);
    _details->setPen(QPen(Qt::white));
    _details->setVisible(_detailsVisible);
    _details->setGroup(this);

    setMarker(marker);

    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

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

void MapMarkerGraphicsItem::setMarker(const MapMarker& marker)
{
    int markerSize = MARKER_SIZE * marker.getIconScale() / 25;
    QImage iconScaled = QImage(marker.getIconFile()).scaled(markerSize, markerSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage iconColored;
    if((!iconScaled.isNull()) && (marker.isColoredIcon()))
    {
        MapColorizeFilter filter;
        QColor filterColor = marker.getColor();

        filter._r2r = 0.33 * filterColor.redF();
        filter._g2r = 0.33 * filterColor.redF();
        filter._b2r = 0.33 * filterColor.redF();
        filter._r2g = 0.33 * filterColor.greenF();
        filter._g2g = 0.33 * filterColor.greenF();
        filter._b2g = 0.33 * filterColor.greenF();
        filter._r2b = 0.33 * filterColor.blueF();
        filter._g2b = 0.33 * filterColor.blueF();
        filter._b2b = 0.33 * filterColor.blueF();

        iconColored = filter.apply(iconScaled);
    }
    else
    {
        iconColored = iconScaled;
    }
    _markerIcon->setPixmap(QPixmap::fromImage(iconColored));
    _markerIcon->setPos(-markerSize / 2, -markerSize);

    _title->setText(marker.getTitle());
    _title->setBrush(QBrush(marker.getColor()));
    QFont titleFont = _title->font();
    titleFont.setPointSize(markerSize / 8);
    _title->setFont(titleFont);

    _details->setText(marker.getDescription());
    _details->setBrush(QBrush(marker.getColor()));
    QFont detailsFont = _details->font();
    detailsFont.setPointSize(markerSize / 10);
    _details->setFont(detailsFont);
    _details->setPos(0, QFontMetrics(detailsFont).height() * 15 / 10);
}

void MapMarkerGraphicsItem::setDetailsVisible(bool visible)
{
    if(_detailsVisible != visible)
    {
        _detailsVisible = visible;
        _details->setVisible(_detailsVisible);
    }
}

int MapMarkerGraphicsItem::getMarkerId() const
{
    return _marker;
}

QVariant MapMarkerGraphicsItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if((change == QGraphicsItem::ItemSelectedChange) && (value.toBool() == false))
    {
        setDetailsVisible(false);
    }
    else if(change == QGraphicsItem::ItemPositionHasChanged)
    {
        _mapFrame.mapMarkerMoved(_marker);
    }

    return QGraphicsItem::itemChange(change, value);
}

void MapMarkerGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    setDetailsVisible(true);
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
    _mapFrame.setPartySelected(false);

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

void MapMarkerGraphicsItem::toggleDetails()
{
    setDetailsVisible(!_detailsVisible);
}

int MapMarkerGraphicsItem::type() const
{
    return Type;
}
