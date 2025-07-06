#include "camerarect.h"
#include "dmconstants.h"
#include "camerascene.h"
#include <QPen>
#include <QCursor>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneHoverEvent>

const qreal CAMERA_RECT_BORDER_SIZE = 4.0;
const int CAMERA_RECT_BORDER_WIDTH = 1;

/*
 * TODOs to finish this
 * changes to dialog need to impact camera when coupled
 * for videos, only restart the player when the camera changes, not when the view changes
 * decent icons and layout
 * DON'T DO: add camera rect to map view
 */
CameraRect::CameraRect(qreal width, qreal height, QGraphicsScene& scene, QWidget* viewport, bool ratioLocked) :
    QGraphicsRectItem (0.0, 0.0, width, height, nullptr),
    _draw(true),
    _mouseDown(false),
    _mouseDownPos(),
    _mouseLastPos(),
    _mouseDownSection(0),
    _drawItem(nullptr),
    _drawText(nullptr),
    _drawTextRect(nullptr),
    _ratioLocked(ratioLocked),
    _sizeLocked(false),
    _viewport(viewport)
{
    initialize(scene);
}

CameraRect::CameraRect(const QRectF& rect, QGraphicsScene& scene, QWidget* viewport, bool ratioLocked) :
    QGraphicsRectItem(rect, nullptr),
    _draw(true),
    _mouseDown(false),
    _mouseDownPos(),
    _mouseLastPos(),
    _mouseDownSection(0),
    _drawItem(nullptr),
    _drawText(nullptr),
    _drawTextRect(nullptr),
    _ratioLocked(ratioLocked),
    _sizeLocked(false),
    _viewport(viewport)
{
    initialize(scene);
    setPos(rect.topLeft());
}

CameraRect::~CameraRect()
{
    delete _drawItem;
}

void CameraRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    return;
}

QRectF CameraRect::getCameraRect() const
{
    return QRectF(pos(), rect().size());
}

void CameraRect::setCameraRect(const QRectF& rect)
{
    setPos(rect.topLeft());
    setRect(0.0, 0.0, rect.width(), rect.height());
    _drawItem->setRect(0.0, 0.0, rect.width(), rect.height());

    CameraScene* cameraScene = dynamic_cast<CameraScene*>(scene());
    if(cameraScene)
        cameraScene->handleItemChanged(this);
}

void CameraRect::setCameraSelectable(bool selectable)
{
    setFlag(QGraphicsItem::ItemIsMovable, selectable);
    setFlag(QGraphicsItem::ItemIsSelectable, selectable);

    setZValue(selectable ? DMHelper::BattleDialog_Z_Overlay : DMHelper::BattleDialog_Z_Camera);
    if((!selectable) && (_viewport))
        _viewport->unsetCursor();
}

void CameraRect::setDraw(bool draw)
{
    _drawItem->setVisible(draw);
}

void CameraRect::setPublishing(bool publishing)
{
    if((!_drawItem) || (!_drawText) || (!_drawTextRect))
        return;

    QColor color = publishing ? QColor(255, 0, 0, 255) : QColor(0, 0, 255, 255);
    QPen p(color);
    _drawItem->setPen(p);
    _drawTextRect->setBrush(QBrush(color));
}

void CameraRect::setRatioLocked(bool locked)
{
    _ratioLocked = locked;
}

void CameraRect::setSizeLocked(bool locked)
{
    _sizeLocked = locked;
}

void CameraRect::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if((!event) || ((flags() & QGraphicsItem::ItemIsSelectable) == 0) || (!_viewport))
        return;

    int section = getRectSection(event->pos());

    if(section == RectSection_Middle)
    {
        _viewport->setCursor(QCursor(Qt::SizeAllCursor));
    }
    else if((section == RectSection_None) || (_sizeLocked))
    {
        _viewport->unsetCursor();
    }
    else
    {
        if((section == RectSection_TopLeft) || (section == RectSection_BottomRight))
            _viewport->setCursor(QCursor(Qt::SizeFDiagCursor));
        else if((section == RectSection_TopRight) || (section == RectSection_BottomLeft))
            _viewport->setCursor(QCursor(Qt::SizeBDiagCursor));
        else if((section == RectSection_Top) || (section == RectSection_Bottom))
            _viewport->setCursor(QCursor(Qt::SizeVerCursor));
        else if((section == RectSection_Left) || (section == RectSection_Right))
            _viewport->setCursor(QCursor(Qt::SizeHorCursor));
        else
            _viewport->unsetCursor();
    }

    QGraphicsRectItem::hoverMoveEvent(event);
}

void CameraRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(_viewport)
        _viewport->unsetCursor();

    QGraphicsRectItem::hoverLeaveEvent(event);
}

void CameraRect::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if((!event) || (!_mouseDown) || (_mouseDownSection == RectSection_None))
    {
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }

    if(_mouseDownSection == RectSection_Middle)
    {
        // Use the normal functionality to move the rectangle around
        QGraphicsRectItem::mouseMoveEvent(event);
    }
    else
    {
        // Resize the rectangle
        qreal dx = 0.0;
        qreal dy = 0.0;
        qreal w = rect().width();
        qreal h = rect().height();

        if(_ratioLocked)
            resizeRectangleFixed(*event, dx, dy, w, h); // Resize the rectangle with a fixed aspect ratio
        else
            resizeRectangle(*event, dx, dy, w, h); // Resize the rectangle normally

        moveBy(dx, dy);
        setRect(0.0, 0.0, w, h);
        _drawItem->setRect(0.0, 0.0, w, h);

        _mouseLastPos = event->pos();
    }

    CameraScene* cameraScene = dynamic_cast<CameraScene*>(scene());
    if(cameraScene)
        cameraScene->handleItemChanged(this);
}

void CameraRect::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    _mouseDown = true;
    _mouseDownPos = event->pos();
    _mouseLastPos = _mouseDownPos;
    _mouseDownSection = getRectSection(_mouseDownPos);

    QGraphicsRectItem::mousePressEvent(event);
}

void CameraRect::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    _mouseDown = false;
    _mouseDownPos = QPointF(0.0, 0.0);
    _mouseLastPos = _mouseDownPos;
    _mouseDownSection = RectSection_None;

    QGraphicsRectItem::mouseReleaseEvent(event);

    CameraScene* cameraScene = dynamic_cast<CameraScene*>(scene());
    if(cameraScene)
        cameraScene->handleItemChanged(this);
}

QVariant CameraRect::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionHasChanged)
        _drawItem->setPos(pos());

    return QGraphicsItem::itemChange(change, value);
}

void CameraRect::initialize(QGraphicsScene& scene)
{
    setZValue(DMHelper::BattleDialog_Z_Camera);

    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    scene.addItem(this);

    _drawItem = new FixedBorderRectItem;
    _drawItem->setRect(0.0, 0.0, rect().width(), rect().height());
    _drawItem->setZValue(DMHelper::BattleDialog_Z_Overlay);
    scene.addItem(_drawItem);

    _drawTextRect = new FixedBorderRectItem(_drawItem);
    _drawTextRect->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    _drawText = new QGraphicsSimpleTextItem(QString(" Player's View "), _drawItem);
    _drawText->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    QFont textFont = _drawText->font();
    textFont.setPointSize(10);
    _drawText->setFont(textFont);
    _drawTextRect->setRect(_drawText->boundingRect().toRect());

    setPublishing(false);
}

int CameraRect::getRectSection(const QPointF point)
{
    if((point.x() < 0) || (point.x() > rect().width()) || (point.y() < 0) || (point.y() > rect().height()))
        return RectSection_None;

    int result = 0;

    if(point.x() < CAMERA_RECT_BORDER_SIZE)
        result |= RectSection_Left;
    else if(point.x() > rect().width() - CAMERA_RECT_BORDER_SIZE)
        result |= RectSection_Right;

    if(point.y() < CAMERA_RECT_BORDER_SIZE)
        result |= RectSection_Top;
    else if(point.y() > rect().height() - CAMERA_RECT_BORDER_SIZE)
        result |= RectSection_Bottom;

    if(result == 0)
        return RectSection_Middle;
    else
        return result;
}

void CameraRect::resizeRectangle(QGraphicsSceneMouseEvent& event, qreal& dx, qreal& dy, qreal& w, qreal& h)
{
    if((_mouseDownSection & RectSection_Left) == RectSection_Left)
    {
        dx = event.pos().x() - _mouseDownPos.x();
        w -= dx;
    }
    else if((_mouseDownSection & RectSection_Right) == RectSection_Right)
    {
        w += event.pos().x() - _mouseLastPos.x();
    }

    if((_mouseDownSection & RectSection_Top) == RectSection_Top)
    {
        dy = event.pos().y() - _mouseDownPos.y();
        h -= dy;
    }
    else if((_mouseDownSection & RectSection_Bottom) == RectSection_Bottom)
    {
        h += event.pos().y() - _mouseLastPos.y();
    }
}

void CameraRect::resizeRectangleFixed(QGraphicsSceneMouseEvent& event, qreal& dx, qreal& dy, qreal& w, qreal& h)
{
    if((_mouseDownSection == RectSection_Left) || (_mouseDownSection == RectSection_Top) || (_mouseDownSection == RectSection_TopLeft))
    {
        if((_mouseDownSection & RectSection_Left) == RectSection_Left)
        {
            dx = event.pos().x() - _mouseDownPos.x();
            w -= dx;
        }
        if((_mouseDownSection & RectSection_Top) == RectSection_Top)
        {
            dy = event.pos().y() - _mouseDownPos.y();
            h -= dy;
        }

        QSizeF newSize = rect().size().scaled(QSizeF(w, h), ((_mouseDownSection == RectSection_TopLeft) || (dx > 0.0) || (dy > 0.0)) ? Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding);
        w = newSize.width();
        h = newSize.height();
        dx = rect().size().width() - w;
        dy = rect().size().height() - h;
    }
    else if(_mouseDownSection == RectSection_TopRight)
    {
        w += event.pos().x() - _mouseLastPos.x();
        dy = event.pos().y() - _mouseDownPos.y();
        h -= dy;

        QSizeF newSize = rect().size().scaled(QSizeF(w, h), Qt::KeepAspectRatio);
        w = newSize.width();
        h = newSize.height();
        dx = 0.0;
        dy = rect().size().height() - h;
    }
    else if((_mouseDownSection == RectSection_Right) || (_mouseDownSection == RectSection_Bottom) || (_mouseDownSection == RectSection_BottomRight))
    {
        if((_mouseDownSection & RectSection_Right) == RectSection_Right)
        {
            w += event.pos().x() - _mouseLastPos.x();
        }
        if((_mouseDownSection & RectSection_Bottom) == RectSection_Bottom)
        {
            h += event.pos().y() - _mouseLastPos.y();
        }

        QSizeF newSize = rect().size().scaled(QSizeF(w, h), ((_mouseDownSection == RectSection_BottomRight) || (dx > 0.0) || (dy > 0.0)) ? Qt::KeepAspectRatio : Qt::KeepAspectRatioByExpanding);
        w = newSize.width();
        h = newSize.height();
        dx = 0;
        dy = 0;
    }
    else if(_mouseDownSection == RectSection_BottomLeft)
    {
        dx = event.pos().x() - _mouseDownPos.x();
        w -= dx;
        h += event.pos().y() - _mouseLastPos.y();

        QSizeF newSize = rect().size().scaled(QSizeF(w, h), Qt::KeepAspectRatio);
        w = newSize.width();
        h = newSize.height();
        dx = rect().size().width() - w;
        dy = 0.0;
    }
}

CameraRect::FixedBorderRectItem::FixedBorderRectItem(QGraphicsItem* parent) :
    QGraphicsRectItem(parent)
{
}

void CameraRect::FixedBorderRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Calculate scale factor from painter transform
    qreal scaleX = painter->transform().m11();
    qreal scaleY = painter->transform().m22();
    qreal scale = (scaleX + scaleY) / 2.0; // Average to be safe

    QPen p = pen();
    p.setWidthF(1.0 / scale); // 1-pixel constant width
    painter->setPen(p);
    painter->setBrush(brush());

    painter->drawRect(rect());
}
