#ifndef GRIDSIZER_H
#define GRIDSIZER_H

#include <QGraphicsItem>

class GridSizer : public QGraphicsItem
{
public:
    GridSizer(qreal size = 250.0, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool isInResizeHandle(const QPointF &pos) const;
    void updateAspectRatioResize(const QPointF &mousePos);

    QRectF rect;
    QPointF dragStart;
    bool resizing = false;
    qreal handleSize = 10.0;
};

#endif // GRIDSIZER_H
