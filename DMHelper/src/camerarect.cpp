#include "camerarect.h"
#include "dmconstants.h"
#include "battledialoggraphicsscene.h"
#include <QPen>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneHoverEvent>

const qreal CAMERA_RECT_BORDER_SIZE = 4.0;
const int CAMERA_RECT_BORDER_WIDTH = 1;
const qreal CAMERA_SHADOW_OFFSET = 4.0;
const int CAMERA_SHADOW_SIZE = 3;

/*
 * TODOs to finish this
 * changes to dialog need to impact camera when coupled
 * for videos, only restart the player when the camera changes, not when the view changes
 * decent icons and layout
 * add camera rect to map view
 */
CameraRect::CameraRect(qreal width, qreal height, QGraphicsScene& scene) :
    QGraphicsRectItem (0.0, 0.0, width, height, nullptr),
    _draw(true),
    _mouseDown(false),
    _mouseDownPos(),
    _mouseLastPos(),
    _mouseDownSection(0),
    _shadowItem(nullptr),
    _drawItem(nullptr),
    _drawText(nullptr),
    _drawTextRect(nullptr)
{
    initialize(scene);
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

    /*
    if(_draw)
    {
        QStyleOptionGraphicsItem myoption = (*option);
        myoption.state &= ~QStyle::State_Selected;
        QGraphicsRectItem::paint(painter, &myoption, widget);
    }*/

}

void CameraRect::setCameraRect(const QRectF& rect)
{
    setPos(rect.topLeft());
    setRect(0.0, 0.0, rect.width(), rect.height());
    _shadowItem->setRect(CAMERA_SHADOW_OFFSET, CAMERA_SHADOW_OFFSET, rect.width(), rect.height());
    _drawItem->setRect(0.0, 0.0, rect.width(), rect.height());

    BattleDialogGraphicsScene* battleScene = dynamic_cast<BattleDialogGraphicsScene*>(scene());
    if(battleScene)
        battleScene->handleItemChanged(this);
}

void CameraRect::setCameraSelectable(bool selectable)
{
    setFlag(QGraphicsItem::ItemIsMovable, selectable);
    setFlag(QGraphicsItem::ItemIsSelectable, selectable);
}

void CameraRect::setDraw(bool draw)
{
    _drawItem->setVisible(draw);
}

void CameraRect::setPublishing(bool publishing)
{
    if((!_drawItem) || (!_shadowItem) || (!_drawText) || (!_drawTextRect))
        return;

    if(publishing)
    {
        _drawItem->setPen(QPen(QColor(255,0,0,255), CAMERA_RECT_BORDER_WIDTH));
        _shadowItem->setPen(QPen(QColor(180,0,0,80), CAMERA_SHADOW_SIZE));
        _drawTextRect->setBrush(QBrush(QColor(255,0,0)));
    }
    else
    {
        _drawItem->setPen(QPen(QColor(0,0,255,255), CAMERA_RECT_BORDER_WIDTH));
        _shadowItem->setPen(QPen(QColor(0,0,180,80), CAMERA_SHADOW_SIZE));
        _drawTextRect->setBrush(QBrush(QColor(0,0,255)));
    }
}

void CameraRect::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if((!event) || ((flags() & QGraphicsItem::ItemIsSelectable) == 0))
    {
        unsetCursor();
        return;
    }

    int section = getRectSection(event->pos());
    switch(section)
    {
        case RectSection_Top:
        case RectSection_Bottom:
            setCursor(QCursor(Qt::SizeVerCursor)); break;
        case RectSection_Left:
        case RectSection_Right:
            setCursor(QCursor(Qt::SizeHorCursor)); break;
        case RectSection_TopLeft:
        case RectSection_BottomRight:
            setCursor(QCursor(Qt::SizeFDiagCursor)); break;
        case RectSection_TopRight:
        case RectSection_BottomLeft:
            setCursor(QCursor(Qt::SizeBDiagCursor)); break;
        case RectSection_Middle:
            setCursor(QCursor(Qt::SizeAllCursor)); break;
        default:
            break;
    }
}

void CameraRect::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if((!_mouseDown) || (_mouseDownSection == RectSection_None) || (_mouseDownSection == RectSection_Middle))
    {
        // Use the normal functionality to move the rectangle around
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }

    // Resize the rectangle
    qreal dx = 0.0;
    qreal dy = 0.0;
    qreal w = rect().width();
    qreal h = rect().height();

    if((_mouseDownSection & RectSection_Left) == RectSection_Left)
    {
        dx = event->pos().x() - _mouseDownPos.x();
        w -= dx;
    }
    else if((_mouseDownSection & RectSection_Right) == RectSection_Right)
    {
        w += event->pos().x() - _mouseLastPos.x();
    }

    if((_mouseDownSection & RectSection_Top) == RectSection_Top)
    {
        dy = event->pos().y() - _mouseDownPos.y();
        h -= dy;
    }
    if((_mouseDownSection & RectSection_Bottom) == RectSection_Bottom)
    {
        h += event->pos().y() - _mouseLastPos.y();
    }

    moveBy(dx, dy);
    setRect(0.0, 0.0, w, h);
    _shadowItem->setRect(CAMERA_SHADOW_OFFSET, CAMERA_SHADOW_OFFSET, w, h);
    _drawItem->setRect(0.0, 0.0, w, h);

    _mouseLastPos = event->pos();

    /*
    BattleDialogGraphicsScene* battleScene = dynamic_cast<BattleDialogGraphicsScene*>(scene());
    if(battleScene)
        battleScene->handleItemChanged(this);
        */
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

    BattleDialogGraphicsScene* battleScene = dynamic_cast<BattleDialogGraphicsScene*>(scene());
    if(battleScene)
        battleScene->handleItemChanged(this);
}

QVariant CameraRect::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionHasChanged)
    {
        _drawItem->setPos(pos());
    }

    return QGraphicsItem::itemChange(change, value);
}

void CameraRect::initialize(QGraphicsScene& scene)
{
    setZValue(DMHelper::BattleDialog_Z_Camera);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    scene.addItem(this);

    _drawItem = new QGraphicsRectItem;
    _drawItem->setRect(0.0, 0.0, rect().width(), rect().height());
    _drawItem->setZValue(DMHelper::BattleDialog_Z_Overlay);
    scene.addItem(_drawItem);

    _shadowItem = new QGraphicsRectItem(_drawItem);
    _shadowItem->setRect(CAMERA_SHADOW_OFFSET, CAMERA_SHADOW_OFFSET, rect().width(), rect().height());
    //_shadowItem->setPen(QPen(QColor(0,0,0,128), 2));
    _shadowItem->setFlag(QGraphicsItem::ItemStacksBehindParent, true);

    _drawTextRect = new QGraphicsRectItem(_drawItem);
    _drawText = new QGraphicsSimpleTextItem(QString(" Player's View "), _drawItem);
    QFont textFont = _drawText->font();
    textFont.setPointSize(5);
    _drawText->setFont(textFont);
    _drawTextRect->setRect(_drawText->boundingRect().toRect());
    //_drawText->setPen(QPen(Qt::black));

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