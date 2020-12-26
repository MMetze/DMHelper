#ifndef UNSELECTEDELLIPSE_H
#define UNSELECTEDELLIPSE_H

#include <QGraphicsEllipseItem>

class UnselectedEllipse : public QGraphicsEllipseItem
{
public:
    UnselectedEllipse(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

#endif // UNSELECTEDELLIPSE_H
