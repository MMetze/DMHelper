#ifndef MAPMARKERGRAPHICSITEM_H
#define MAPMARKERGRAPHICSITEM_H

#include "mapcontent.h"
#include <QGraphicsItemGroup>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>

class QGraphicsScene;
class MapFrame;
class MapMarkerGraphicsPixmapItem;
class MapMarkerGraphicsSimpleTextItem;
class UndoMarker;

class MapMarkerGraphicsItem : public QGraphicsItemGroup
{
public:
//    MapMarkerGraphicsItem(QGraphicsScene* scene, const MapMarker& marker, qreal initialScale/*, MapFrame& mapFrame*/);
    MapMarkerGraphicsItem(QGraphicsScene* scene, UndoMarker* marker, qreal initialScale);

    enum { Type = UserType + 1 };

    void setGroupVisible(bool visible);
    void setMarker(const MapMarker& marker);
    void setDetailsVisible(bool visible);

//    int getMarkerId() const;

    void drawGraphicsItem(QPainter& painter);
    QPixmap getGraphicsItemPixmap() const;
    QPointF getTopLeft() const;

    UndoMarker* getMarker();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;
    void toggleDetails();
    void preparePixmap();

    virtual int type() const override;

    //int _marker;
    //MapFrame& _mapFrame;
    UndoMarker* _marker;

    MapMarkerGraphicsPixmapItem* _markerIcon;
    MapMarkerGraphicsSimpleTextItem* _title;
    MapMarkerGraphicsSimpleTextItem* _details;
    QPixmap _markerPixmap;

    bool _detailsVisible;
    bool _clicked;
};

class MapMarkerGraphicsPixmapItem : public QGraphicsPixmapItem
{
public:
    MapMarkerGraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent) :
        QGraphicsPixmapItem(pixmap, parent)
    {}

    MapMarkerGraphicsPixmapItem(QGraphicsItem *parent) :
        QGraphicsPixmapItem(parent)
    {}

protected:
    virtual int type() const { return MapMarkerGraphicsItem::Type; }
};

class MapMarkerGraphicsSimpleTextItem : public QGraphicsSimpleTextItem
{
public:
    MapMarkerGraphicsSimpleTextItem(const QString &text, QGraphicsItem *parent) :
        QGraphicsSimpleTextItem(text, parent)
    {}

    MapMarkerGraphicsSimpleTextItem(QGraphicsItem *parent) :
        QGraphicsSimpleTextItem(parent)
    {}

protected:
    virtual int type() const { return MapMarkerGraphicsItem::Type; }
};

#endif // MAPMARKERGRAPHICSITEM_H
