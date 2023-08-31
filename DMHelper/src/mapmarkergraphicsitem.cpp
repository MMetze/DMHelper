#include "mapmarkergraphicsitem.h"
#include "mapframe.h"
#include "scaledpixmap.h"
#include "mapcolorizefilter.h"
#include "undomarker.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QPainter>

const int MARKER_SIZE = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0];

//MapMarkerGraphicsItem::MapMarkerGraphicsItem(QGraphicsScene* scene, const MapMarker& marker, qreal initialScale/*, MapFrame& mapFrame*/) :
MapMarkerGraphicsItem::MapMarkerGraphicsItem(QGraphicsScene* scene, UndoMarker* marker, qreal initialScale) :
    QGraphicsItemGroup(),
    _marker(marker),
//    _marker(marker.getID()),
//    _mapFrame(mapFrame),
    _markerIcon(nullptr),
    _title(nullptr),
    _details(nullptr),
    _markerPixmap(),
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

    setScale(initialScale);
    if(_marker)
        setMarker(_marker->getMarker());

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

    preparePixmap();
}

void MapMarkerGraphicsItem::setDetailsVisible(bool visible)
{
    if(_detailsVisible != visible)
    {
        _detailsVisible = visible;
        _details->setVisible(_detailsVisible);
    }
}

/*
int MapMarkerGraphicsItem::getMarkerId() const
{
    return _marker;
}
*/

void MapMarkerGraphicsItem::drawGraphicsItem(QPainter& painter)
{
    if(!_markerPixmap.isNull())
        painter.drawPixmap(_markerIcon->scenePos(), _markerPixmap);
}

QPixmap MapMarkerGraphicsItem::getGraphicsItemPixmap() const
{
    return _markerPixmap;
}

QPointF MapMarkerGraphicsItem::getTopLeft() const
{
    return _markerIcon->pos() * scale();
}

UndoMarker* MapMarkerGraphicsItem::getMarker()
{
    return _marker;
}

QVariant MapMarkerGraphicsItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if((change == QGraphicsItem::ItemSelectedChange) && (value.toBool() == false))
        setDetailsVisible(false);
    else if((change == QGraphicsItem::ItemPositionHasChanged) && (_marker))
        _marker->moveMapMarker(); //_marker.mapMarkerMoved();

    return QGraphicsItem::itemChange(change, value);
}

void MapMarkerGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    setDetailsVisible(true);
    //_mapFrame.editMapMarker(_marker);
    if(_marker)
        _marker->editMapMarker();

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
    //_mapFrame.setPartySelected(false);
    if(_marker)
        _marker->unsetPartySelected();

    QGraphicsItem::mousePressEvent(event);
}

void MapMarkerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if(_clicked)
    {
        if((event) && ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) && (_marker))
            _marker->activateMapMarker(); //_mapFrame.activateMapMarker(_marker);
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

void MapMarkerGraphicsItem::preparePixmap()
{
    _markerPixmap = QPixmap();
    if((!_markerIcon) || (!_title))
        return;

    QSize pixmapSize = boundingRect().size().toSize();
    QSize iconSize = _markerIcon->boundingRect().size().toSize();
    QSize titleSize = _title->boundingRect().size().toSize();
    qreal pixmapScale = scale();

    pixmapSize = QSize((iconSize.width() / 2) + titleSize.width(),
                       iconSize.height() + titleSize.height());

    _markerPixmap = QPixmap(pixmapSize);
    _markerPixmap.fill(Qt::transparent);

    QPainter itemPainter(&_markerPixmap);
    itemPainter.setRenderHint(QPainter::Antialiasing);
    itemPainter.scale(pixmapScale, pixmapScale);
    QStyleOptionGraphicsItem opt;
    _markerIcon->paint(&itemPainter, &opt, nullptr);
    itemPainter.translate(iconSize.width() / 2, iconSize.height());
    _title->paint(&itemPainter, &opt, nullptr);
}

int MapMarkerGraphicsItem::type() const
{
    return Type;
}
