#ifndef UNSELECTEDPOLYGON_H
#define UNSELECTEDPOLYGON_H

#include <QGraphicsPolygonItem>

class UnselectedPolygon : public QGraphicsPolygonItem
{
public:
    UnselectedPolygon(const QPolygonF &polygon, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

};

#endif // UNSELECTEDPOLYGON_H
