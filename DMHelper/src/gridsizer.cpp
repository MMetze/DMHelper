#include "gridsizer.h"
#include "dmconstants.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

const int GRID_COUNT = 5;
const qreal MINIMUM_GRID_SIZE = 10.0;

GridSizer::GridSizer(qreal size, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    _gridSizerRect(0, 0, size * GRID_COUNT, size * GRID_COUNT),
    _resizing(false),
    _mouseDownPos(),
    _gridSize(size)
{
    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
    setZValue(DMHelper::BattleDialog_Z_Overlay);

    setSize(size);
}

QRectF GridSizer::boundingRect() const
{
    return _gridSizerRect.adjusted(-getHandleSize(), -getHandleSize(), getHandleSize(), getHandleSize());
}

void GridSizer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Draw semi-transparent white background
    QColor backgroundColor(255, 255, 255, 128); // RGBA: 50% transparent white
    painter->setBrush(backgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(_gridSizerRect);

    QPen pen(Qt::black);
    pen.setWidth(1);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // Draw grid (5x5)
    qreal step = _gridSizerRect.width() / GRID_COUNT;
    for (int i = 0; i <= GRID_COUNT; ++i)
    {
        qreal pos = _gridSizerRect.left() + i * step;
        painter->drawLine(QPointF(pos, _gridSizerRect.top()), QPointF(pos, _gridSizerRect.bottom()));
        painter->drawLine(QPointF(_gridSizerRect.left(), pos), QPointF(_gridSizerRect.right(), pos));
    }

    // Draw resize handle (bottom-right corner)
    painter->setBrush(Qt::gray);
    painter->drawRect(_gridSizerRect.right() - getHandleSize(), _gridSizerRect.bottom() - getHandleSize(), getHandleSize(), getHandleSize());
}

qreal GridSizer::getSize() const
{
    return _gridSize;
}

qreal GridSizer::getHandleSize() const
{
    return _gridSize / (10 / GRID_COUNT);
}

void GridSizer::setSize(qreal size)
{
    _gridSize = qMax(size, MINIMUM_GRID_SIZE); // Set the size, considering a minimum

    prepareGeometryChange();  // Tells the scene the bounding _gridSizerRect will change

    _gridSizerRect.setSize(QSizeF(_gridSize * GRID_COUNT, _gridSize * GRID_COUNT)); // Maintain square aspect ratio
}

bool GridSizer::isInResizeHandle(const QPointF &pos) const
{
    QRectF handle(_gridSizerRect.right() - getHandleSize(), _gridSizerRect.bottom() - getHandleSize(), getHandleSize(), getHandleSize());
    return handle.contains(pos);
}

void GridSizer::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(isInResizeHandle(event->pos()))
    {
        _resizing = true;
        _mouseDownPos = _gridSizerRect.bottomRight() - event->pos();
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void GridSizer::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (_resizing)
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
    _resizing = false;
    QGraphicsItem::mouseReleaseEvent(event);
}

void GridSizer::updateAspectRatioResize(const QPointF &mousePos)
{
//    qreal delta = qMax(mousePos.x() - _mouseDownPos.x(), mousePos.y() - _mouseDownPos.y());
//    delta = qMax(delta, 50.0); // Minimum size

//    rect.setWidth(delta);
//    rect.setHeight(delta); // Maintain square aspect ratio
setSize((qMax(mousePos.x() - _gridSizerRect.left() + _mouseDownPos.x(), mousePos.y() - _gridSizerRect.top() + _mouseDownPos.y())) / GRID_COUNT);

//    setSize((qMax(_gridSizerRect.width() + mouseDelta.x(), _gridSizerRect.height() + mouseDelta.y())) / GRID_COUNT);
}

