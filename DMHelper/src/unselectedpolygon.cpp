#include "unselectedpolygon.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>

UnselectedPolygon::UnselectedPolygon(const QPolygonF &polygon, QGraphicsItem *parent) :
    QGraphicsPolygonItem(polygon, parent)
{
}

void UnselectedPolygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsPolygonItem::paint(painter, &myoption, widget);
}
