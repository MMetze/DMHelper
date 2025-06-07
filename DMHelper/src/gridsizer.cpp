#include "gridsizer.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

GridSizer::GridSizer(qreal size, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    rect(0, 0, size, size)
{
    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
}

QRectF GridSizer::boundingRect() const
{
    return rect.adjusted(-handleSize, -handleSize, handleSize, handleSize);
}

void GridSizer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Draw semi-transparent white background
    QColor backgroundColor(255, 255, 255, 128); // RGBA: 50% transparent white
    painter->setBrush(backgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);

    QPen pen(Qt::black);
    pen.setWidth(1);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Draw grid (5x5)
    int gridSize = 5;
    qreal step = rect.width() / gridSize;
    for (int i = 0; i <= gridSize; ++i)
    {
        qreal pos = rect.left() + i * step;
        painter->drawLine(QPointF(pos, rect.top()), QPointF(pos, rect.bottom()));
        painter->drawLine(QPointF(rect.left(), pos), QPointF(rect.right(), pos));
    }

    // Draw resize handle (bottom-right corner)
    painter->setBrush(Qt::gray);
    painter->drawRect(rect.right() - handleSize, rect.bottom() - handleSize, handleSize, handleSize);
}

bool GridSizer::isInResizeHandle(const QPointF &pos) const
{
    QRectF handle(rect.right() - handleSize, rect.bottom() - handleSize, handleSize, handleSize);
    return handle.contains(pos);
}

void GridSizer::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (isInResizeHandle(event->pos()))
    {
        resizing = true;
        dragStart = event->pos();
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void GridSizer::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (resizing)
    {
        updateAspectRatioResize(event->pos());
        update();
    }
    else
    {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void GridSizer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    resizing = false;
    QGraphicsItem::mouseReleaseEvent(event);
}

void GridSizer::updateAspectRatioResize(const QPointF &mousePos)
{
    QPointF bottomRight = rect.bottomRight();
    qreal delta = qMax(mousePos.x() - rect.left(), mousePos.y() - rect.top());
    delta = qMax(delta, 50.0); // Minimum size

    rect.setWidth(delta);
    rect.setHeight(delta); // Maintain square aspect ratio
}

