#ifndef UNSELECTEDRECT_H
#define UNSELECTEDRECT_H

#include <QGraphicsRectItem>

class UnselectedRect : public QGraphicsRectItem
{
public:
    UnselectedRect(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

};

#endif // UNSELECTEDRECT_H
