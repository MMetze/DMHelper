#include "unselectedrect.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>

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
