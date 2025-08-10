#include "gridsizer.h"
#include "dmconstants.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

const int GRID_COUNT = 5;
const qreal MINIMUM_GRID_SIZE = 10.0;

GridSizer::GridSizer(qreal size, bool showAccept, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    _gridSizerRect(0, 0, size * GRID_COUNT, size * GRID_COUNT),
    _resizing(false),
    _mouseDownPos(),
    _gridSize(size),
    _penColor(Qt::black),
    _penWidth(1),
    _backgroundColor(Qt::white),
    _acceptPixmap(),
    _rejectPixmap()
{
    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
    setZValue(DMHelper::BattleDialog_Z_Overlay);

    if(showAccept)
    {
        _acceptPixmap = QPixmap(":/img/data/icon_check.png");
        _rejectPixmap = QPixmap(":/img/data/icon_remove.png");
    }

    setSize(size);
}

QRectF GridSizer::boundingRect() const
{
    return _gridSizerRect.adjusted(-getHandleSize(), -getHandleSize(), getHandleSize(), getHandleSize());
}

void GridSizer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Draw semi-transparent white background
    QColor backgroundColor(_backgroundColor.red(), _backgroundColor.blue(), _backgroundColor.green(), 128); // RGBA: 50% transparent white
    painter->setBrush(backgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(_gridSizerRect);

    QPen pen(_penColor);
    pen.setWidth(_penWidth);
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
    painter->setBrush(_backgroundColor);
    painter->drawRect(_gridSizerRect.right() - (0.75 * getHandleSize()), _gridSizerRect.bottom() - (0.75 * getHandleSize()), getHandleSize(), getHandleSize());

    if(!_acceptPixmap.isNull())
    {
        painter->drawRect(_gridSizerRect.right() - (2.2 * getHandleSize()), _gridSizerRect.top(), getHandleSize(), getHandleSize());
        painter->drawPixmap(_gridSizerRect.right() - (2.2 * getHandleSize()), _gridSizerRect.top(), getHandleSize(), getHandleSize(), _acceptPixmap);
        painter->drawRect(_gridSizerRect.right() - (1.0 * getHandleSize()), _gridSizerRect.top(), getHandleSize(), getHandleSize());
        painter->drawPixmap(_gridSizerRect.right() - (1.0 * getHandleSize()), _gridSizerRect.top(), getHandleSize(), getHandleSize(), _rejectPixmap);
    }
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
    qreal newSize = qMax(size, MINIMUM_GRID_SIZE); // Set the size, considering a minimum

    if(newSize == _gridSize)
        return; // No change, do not update

    _gridSize = newSize;

    prepareGeometryChange();  // Tells the scene the bounding _gridSizerRect will change
    _gridSizerRect.setSize(QSizeF(_gridSize * GRID_COUNT, _gridSize * GRID_COUNT)); // Maintain square aspect ratio
}

bool GridSizer::isInResizeHandle(const QPointF &pos) const
{
    QRectF handle(_gridSizerRect.right() - (0.75 * getHandleSize()), _gridSizerRect.bottom() - (0.75 * getHandleSize()), getHandleSize(), getHandleSize());
    return handle.contains(pos);
}

bool GridSizer::isInAcceptButton(const QPointF &pos) const
{
    QRectF handle(_gridSizerRect.right() - (2.2 * getHandleSize()), _gridSizerRect.top(), getHandleSize(), getHandleSize());
    return handle.contains(pos);
}

bool GridSizer::isInRejectButton(const QPointF &pos) const
{
    QRectF handle(_gridSizerRect.right() - (1.0 * getHandleSize()), _gridSizerRect.top(), getHandleSize(), getHandleSize());
    return handle.contains(pos);
}

void GridSizer::setPenColor(const QColor &color)
{
    if(color == _penColor)
        return;

    _penColor = color;
    update();
}

void GridSizer::setPenWidth(int width)
{
    if((width == _penWidth) || (width <= 0))
        return;

    _penWidth = width;
    update();
}

void GridSizer::setBackgroundColor(const QColor &color)
{
    if(color == _backgroundColor)
        return;

    _backgroundColor = color;
    update();
}

void GridSizer::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(isInResizeHandle(event->pos()))
    {
        _resizing = true;
        _mouseDownPos = _gridSizerRect.bottomRight() - event->pos();
    }
    else if((isInAcceptButton(event->pos())) || (isInRejectButton(event->pos())))
    {
        _mouseDownPos = event->pos();
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void GridSizer::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(_resizing)
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

    if((isInAcceptButton(event->pos())) || (isInAcceptButton(_mouseDownPos)))
    {
        emit accepted();
    }
    else if((isInRejectButton(event->pos())) || (isInRejectButton(_mouseDownPos)))
    {
        emit rejected();
    }
    else
    {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void GridSizer::updateAspectRatioResize(const QPointF &mousePos)
{
    setSize((qMax(mousePos.x() - _gridSizerRect.left() + _mouseDownPos.x(), mousePos.y() - _gridSizerRect.top() + _mouseDownPos.y())) / GRID_COUNT);
}

