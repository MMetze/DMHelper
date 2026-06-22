#include "layerdrawengine.h"
#include "layerdrawtooldialog.h"
#include "layerdraw.h"
#include "layerdrawshape.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QInputDialog>
#include <QKeyEvent>
#include <QCursor>

LayerDrawEngine::LayerDrawEngine(QObject *parent) :
    QObject{parent},
    _drawLayer{nullptr},
    _toolDialog{nullptr},
    _mouseDownPos{},
    _activeTool{DMHelper::DrawToolType_Path},
    _activeDrawObject{nullptr},
    _activeItem{nullptr}
{}

void LayerDrawEngine::setDrawLayer(LayerDraw* drawLayer)
{
    _drawLayer = drawLayer;
}

void LayerDrawEngine::setActive(bool active)
{
    if(active)
    {
        if(!_toolDialog)
        {
            QWidget* parentWidget = qobject_cast<QWidget*>(parent());
            _toolDialog = new LayerDrawToolDialog(parentWidget ? parentWidget->window() : nullptr);
            connect(_toolDialog, &LayerDrawToolDialog::toolTypeChanged, this, &LayerDrawEngine::handleToolTypeChanged);
        }

        _toolDialog->show();
        handleToolTypeChanged(_toolDialog->getToolType());
    }
    else
    {
        if(_toolDialog)
            _toolDialog->hide();

        _activeItem = nullptr;
    }
}

void LayerDrawEngine::handleMouseDown(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    if((!_drawLayer) || (!_toolDialog) || (buttons == Qt::NoButton))
        return;

    _mouseDownPos = pos;
    _activeTool = _toolDialog->getToolType();

    if(_activeDrawObject)
        discardActiveObject();

    switch(_activeTool)
    {
        case DMHelper::DrawToolType_Path:
            handlePathMouseDown(pos);
            break;
        case DMHelper::DrawToolType_Line:
            handleLineMouseDown(pos);
            break;
        case DMHelper::DrawToolType_Rect:
            handleRectMouseDown(pos);
            break;
        case DMHelper::DrawToolType_Ellipse:
            handleEllipseMouseDown(pos);
            break;
        case DMHelper::DrawToolType_Text:
            handleTextMouseDown(pos);
            break;
        case DMHelper::DrawToolType_Eraser:
            handleEraserMouseDown(pos);
            break;
    }
}

void LayerDrawEngine::handleMouseMoved(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    if((!_drawLayer) || (buttons == Qt::NoButton))
        return;

    switch(_activeTool)
    {
        case DMHelper::DrawToolType_Path:
            handlePathMouseMoved(pos);
            break;
        case DMHelper::DrawToolType_Line:
            handleLineMouseMoved(pos);
            break;
        case DMHelper::DrawToolType_Rect:
            handleRectMouseMoved(pos);
            break;
        case DMHelper::DrawToolType_Ellipse:
            handleEllipseMouseMoved(pos);
            break;
        default:
            break;
    }
}

void LayerDrawEngine::handleMouseUp(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(pos);
    Q_UNUSED(buttons);
    Q_UNUSED(modifiers);

    if(!_activeDrawObject)
        return;

    if(!_activeItem)
    {
        discardActiveObject();
        return;
    }

    commitActiveObject();
}

bool LayerDrawEngine::handleKeyPress(QKeyEvent* event)
{
    if((!event) || (!_drawLayer))
        return false;

    if(event->key() == Qt::Key_Delete)
    {
        _drawLayer->deleteSelectedObjects();
        return true;
    }

    return false;
}

// --- Path tool ---

void LayerDrawEngine::handlePathMouseDown(const QPointF& pos)
{
    _activeDrawObject = new LayerDrawObjectPath(pos, _toolDialog->getToolLineColor(), _toolDialog->getToolLineWidth(), _toolDialog->getToolLineType());
}

void LayerDrawEngine::handlePathMouseMoved(const QPointF& pos)
{
    LayerDrawObjectPath* pathObject = dynamic_cast<LayerDrawObjectPath*>(_activeDrawObject);
    if(!pathObject)
        return;

    pathObject->addPoint(pos);

    if(!_activeItem)
    {
        _activeItem = _drawLayer->createGraphicsItem(_activeDrawObject);
    }
    else
    {
        QGraphicsPathItem* pathItem = dynamic_cast<QGraphicsPathItem*>(_activeItem);
        if(!pathItem)
            return;

        QPainterPath path = pathItem->path();
        path.lineTo(pos);
        pathItem->setPath(path);
    }
}

// --- Line tool ---

void LayerDrawEngine::handleLineMouseDown(const QPointF& pos)
{
    _activeDrawObject = new LayerDrawObjectLine(pos, pos, _toolDialog->getToolLineColor(), _toolDialog->getToolLineWidth(), _toolDialog->getToolLineType());
}

void LayerDrawEngine::handleLineMouseMoved(const QPointF& pos)
{
    LayerDrawObjectLine* lineObject = dynamic_cast<LayerDrawObjectLine*>(_activeDrawObject);
    if(!lineObject)
        return;

    lineObject->setEndPoint(pos);

    if(!_activeItem)
    {
        _activeItem = _drawLayer->createGraphicsItem(_activeDrawObject);
    }
    else
    {
        QGraphicsLineItem* lineItem = dynamic_cast<QGraphicsLineItem*>(_activeItem);
        if(!lineItem)
            return;

        lineItem->setLine(QLineF(lineObject->getStartPoint(), pos));
    }
}

// --- Rect tool ---

void LayerDrawEngine::handleRectMouseDown(const QPointF& pos)
{
    QRectF rect(pos, QSizeF(0.0, 0.0));
    bool filled = _toolDialog->isToolFilled();
    QColor fillColor = filled ? _toolDialog->getToolFillColor() : Qt::transparent;
    _activeDrawObject = new LayerDrawObjectRect(rect, _toolDialog->getToolLineColor(), _toolDialog->getToolLineWidth(), _toolDialog->getToolLineType(), fillColor, filled);
}

void LayerDrawEngine::handleRectMouseMoved(const QPointF& pos)
{
    LayerDrawObjectRect* rectObject = dynamic_cast<LayerDrawObjectRect*>(_activeDrawObject);
    if(!rectObject)
        return;

    QRectF rect = QRectF(_mouseDownPos, pos).normalized();
    rectObject->setRect(rect);

    if(!_activeItem)
    {
        _activeItem = _drawLayer->createGraphicsItem(_activeDrawObject);
    }
    else
    {
        QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(_activeItem);
        if(!rectItem)
            return;

        rectItem->setRect(rect);
    }
}

// --- Ellipse tool ---

void LayerDrawEngine::handleEllipseMouseDown(const QPointF& pos)
{
    QRectF rect(pos, QSizeF(0.0, 0.0));
    bool filled = _toolDialog->isToolFilled();
    QColor fillColor = filled ? _toolDialog->getToolFillColor() : Qt::transparent;
    _activeDrawObject = new LayerDrawObjectEllipse(rect, _toolDialog->getToolLineColor(), _toolDialog->getToolLineWidth(), _toolDialog->getToolLineType(), fillColor, filled);
}

void LayerDrawEngine::handleEllipseMouseMoved(const QPointF& pos)
{
    LayerDrawObjectEllipse* ellipseObject = dynamic_cast<LayerDrawObjectEllipse*>(_activeDrawObject);
    if(!ellipseObject)
        return;

    QRectF rect = QRectF(_mouseDownPos, pos).normalized();
    ellipseObject->setRect(rect);

    if(!_activeItem)
    {
        _activeItem = _drawLayer->createGraphicsItem(_activeDrawObject);
    }
    else
    {
        QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(_activeItem);
        if(!ellipseItem)
            return;

        ellipseItem->setRect(rect);
    }
}

// --- Text tool ---

void LayerDrawEngine::handleTextMouseDown(const QPointF& pos)
{
    bool ok = false;
    QString text = QInputDialog::getText(nullptr, QString("Text"), QString("Enter text:"), QLineEdit::Normal, QString(), &ok);
    if(!ok || text.isEmpty())
        return;

    LayerDrawObjectText* textObject = new LayerDrawObjectText(pos, text, _toolDialog->getToolLineColor(), _toolDialog->getToolFontFamily(), _toolDialog->getToolFontSize());
    _activeDrawObject = textObject;
    _activeItem = _drawLayer->createGraphicsItem(_activeDrawObject);

    commitActiveObject();
}

// --- Eraser tool ---

void LayerDrawEngine::handleEraserMouseDown(const QPointF& pos)
{
    if(!_drawLayer)
        return;

    _drawLayer->eraseObjectAtPosition(pos);
}

void LayerDrawEngine::handleToolTypeChanged(DMHelper::DrawToolType toolType)
{
    if(toolType == DMHelper::DrawToolType_Eraser)
    {
        QPixmap erasePixmap(":/img/data/icon_erase.png");
        int cursorSize = DMHelper::CURSOR_SIZE / 2;
        QPixmap scaled = erasePixmap.scaled(cursorSize, cursorSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        emit cursorChanged(QCursor(scaled, cursorSize / 2, cursorSize / 2));
    }
    else
    {
        QPixmap drawPixmap(":/img/data/icon_edit.png");
        int cursorSize = DMHelper::CURSOR_SIZE / 2;
        QPixmap scaled = drawPixmap.scaled(cursorSize, cursorSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        emit cursorChanged(QCursor(scaled,
                                   58 * cursorSize / drawPixmap.width(),
                                   350 * cursorSize / drawPixmap.height()));
    }
}

// --- Helpers ---

void LayerDrawEngine::commitActiveObject()
{
    if((!_activeDrawObject) || (!_drawLayer))
        return;

    _drawLayer->addObject(_activeDrawObject);
    _activeDrawObject = nullptr;
    _activeItem = nullptr;
}

void LayerDrawEngine::discardActiveObject()
{
    if(_activeItem)
    {
        if(_activeItem->scene())
            _activeItem->scene()->removeItem(_activeItem);
        delete _activeItem;
        _activeItem = nullptr;
    }

    delete _activeDrawObject;
    _activeDrawObject = nullptr;
}
