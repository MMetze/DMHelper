#include "unselectedpolygon.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPen>

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

QVariant UnselectedPolygon::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemSelectedHasChanged)
    {
        QPen itemPen = pen();
        itemPen.setWidth(isSelected() ? 3 : 1);
        setPen(itemPen);
    }

    return QGraphicsItem::itemChange(change, value);
}
