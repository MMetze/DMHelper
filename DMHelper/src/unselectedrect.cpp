#include "unselectedrect.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPen>

UnselectedRect::UnselectedRect(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent) :
    QGraphicsRectItem(x, y, width, height, parent)
{
}

void UnselectedRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsRectItem::paint(painter, &myoption, widget);
}

QVariant UnselectedRect::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemSelectedHasChanged)
    {
        QPen itemPen = pen();
        itemPen.setWidth(isSelected() ? 3 : 1);
        setPen(itemPen);
    }

    return QGraphicsItem::itemChange(change, value);
}
