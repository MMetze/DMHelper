#ifndef GRIDSIZER_H
#define GRIDSIZER_H

#include <QGraphicsItem>

class GridSizer : public QGraphicsItem
{
public:
    GridSizer(qreal size = 250.0, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    qreal getSize() const;
    qreal getHandleSize() const;

public slots:
    void setSize(qreal size);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool isInResizeHandle(const QPointF &pos) const;
    void updateAspectRatioResize(const QPointF &mousePos);

    QRectF _gridSizerRect;
    bool _resizing;
    QPointF _mouseDownPos;
    qreal _gridSize;
};

#endif // GRIDSIZER_H
