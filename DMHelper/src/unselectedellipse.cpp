#include "unselectedellipse.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>

UnselectedEllipse::UnselectedEllipse(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent) :
    QGraphicsEllipseItem(x, y, width, height, parent)
{
}

void UnselectedEllipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsEllipseItem::paint(painter, &myoption, widget);
}
