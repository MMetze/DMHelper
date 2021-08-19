#ifndef MAPMARKERGRAPHICSITEM_H
#define MAPMARKERGRAPHICSITEM_H

#include "mapcontent.h"
#include <QGraphicsItemGroup>

class QGraphicsTextItem;
class QGraphicsScene;
class MapFrame;

class MapMarkerGraphicsItem : public QGraphicsItemGroup
{
public:
    MapMarkerGraphicsItem(QGraphicsScene* scene, const MapMarker& marker, MapFrame& mapFrame);

    enum { Type = UserType + 1 };

    void setGroupVisible(bool visible);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void toggleDetails();

    int type() const;

    int _marker;
    MapFrame& _mapFrame;

    QGraphicsPixmapItem* _markerIcon;
    QGraphicsSimpleTextItem* _title;
    QGraphicsSimpleTextItem* _details;

    bool _detailsVisible;
    bool _clicked;
};

#endif // MAPMARKERGRAPHICSITEM_H
