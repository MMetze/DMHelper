#include "battleframemapdrawer.h"
#include "undofowpath.h"
#include "undofowfill.h"
#include "undofowshape.h"
#include "undofowpolygon.h"
#include "layerfow.h"
#include "layerscene.h"
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>

BattleFrameMapDrawer::BattleFrameMapDrawer(QObject *parent) :
    QObject(parent),
    _mouseDown(false),
    _mouseDownPos(),
    _undoPath(nullptr),
    _scene(nullptr),
    _cursor(),
    _gridScale(10),
    _zoomScale(1.f),
    _size(10),
    _erase(true),
    _smooth(true),
    _brushMode(DMHelper::BrushType_Circle),
    _selectActive(false)

{
    createCursor();
}

void BattleFrameMapDrawer::setScene(LayerScene* scene)
{
    _scene = scene;
}

LayerScene* BattleFrameMapDrawer::getScene() const
{
    return _scene;
}

const QCursor& BattleFrameMapDrawer::getCursor() const
{
    return _cursor;
}

void BattleFrameMapDrawer::handleMouseDown(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    if(!_scene)
        return;

    if(_brushMode == DMHelper::BrushType_Polygon)
    {
        if(buttons & Qt::RightButton)
        {
            applyPolygon();
        }
        else if(buttons & Qt::LeftButton)
        {
            _polygonPoints.append(pos.toPoint());
            emit polygonChanged(QPolygonF(_polygonPoints));
        }
        return;
    }

    if(_brushMode == DMHelper::BrushType_Select)
    {
        if(buttons & Qt::LeftButton)
        {
            _mouseDownPos = pos;
            _selectActive = true;
        }
        return;
    }

    _mouseDownPos = pos;
    _mouseDown = true;

    // Math says divide by 10: radius of 5 to adjust scale to "one square"
    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        _undoPath = new UndoFowPath(layer, MapDrawPath(_gridScale * _size / 10, _brushMode, _erase, _smooth, pos.toPoint() - layer->getPosition()));
        layer->getUndoStack()->push(_undoPath);
        emit dirty();
    }
}

void BattleFrameMapDrawer::handleMouseMoved(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(buttons);
    Q_UNUSED(modifiers);

    if(_brushMode == DMHelper::BrushType_Polygon)
        return;

    if(_brushMode == DMHelper::BrushType_Select)
    {
        if(_selectActive)
            emit selectRectChanged(QRectF(_mouseDownPos, pos).normalized());
        return;
    }

    if((!_undoPath) || (!_undoPath->getLayer()))
        return;

    _undoPath->addPoint(pos.toPoint() - _undoPath->getLayer()->getPosition());
    emit dirty();
}

void BattleFrameMapDrawer::handleMouseUp(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(pos);
    Q_UNUSED(buttons);
    Q_UNUSED(modifiers);

    if(_brushMode == DMHelper::BrushType_Polygon)
        return;

    if(_brushMode == DMHelper::BrushType_Select)
    {
        if(_selectActive)
        {
            QRect selectRect = QRectF(_mouseDownPos, pos).normalized().toRect();
            drawRect(selectRect);
            _selectActive = false;
            emit selectRectCancelled();
        }
        return;
    }

    endPath();
    emit dirty();
}

void BattleFrameMapDrawer::drawRect(const QRect& rect)
{
    if(!_scene)
        return;

    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowShape* undoShape = new UndoFowShape(layer, MapEditShape(rect.translated(-layer->getPosition()), _erase, false));
        layer->getUndoStack()->push(undoShape);
        emit dirty();
    }
    emit dirty();
}

void BattleFrameMapDrawer::setSize(int size)
{
    if(_size == size)
        return;

    _size = size;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::setScale(int gridScale, qreal zoomScale)
{
    if((_gridScale == gridScale) && (_zoomScale == zoomScale))
        return;

    _gridScale = gridScale;
    _zoomScale = zoomScale;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::fillFoW()
{
    if(_erase)
        clearFoW();
    else
        resetFoW();
}

void BattleFrameMapDrawer::resetFoW()
{
    if(!_scene)
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Fill FoW"), QString("Are you sure you would like to fill the entire Fog of War?")) == QMessageBox::No)
        return;

    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowFill* undoFill = new UndoFowFill(layer, MapEditFill(QColor(0, 128, 0, 255)));
        layer->getUndoStack()->push(undoFill);
        emit dirty();
    }
}

void BattleFrameMapDrawer::clearFoW()
{
    if(!_scene)
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Clear FoW"), QString("Are you sure you would like to clear the entire Fog of War?")) == QMessageBox::No)
        return;

    LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
    if(layer)
    {
        UndoFowFill* undoFill = new UndoFowFill(layer, MapEditFill(QColor(0, 128, 0, 0)));
        layer->getUndoStack()->push(undoFill);
        emit dirty();
    }
}

void BattleFrameMapDrawer::setErase(bool erase)
{
    if(_erase == erase)
        return;

    _erase = erase;
}

void BattleFrameMapDrawer::setSmooth(bool smooth)
{
    if(_smooth == smooth)
        return;

    _smooth = smooth;
}

void BattleFrameMapDrawer::setBrushMode(int brushMode)
{
    if(_brushMode == brushMode)
        return;

    if(_brushMode == DMHelper::BrushType_Polygon && !_polygonPoints.isEmpty())
        cancelPolygon();
    if(_brushMode == DMHelper::BrushType_Select && _selectActive)
        cancelSelect();

    _brushMode = brushMode;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::endPath()
{
    _undoPath = nullptr;
    _mouseDown = false;
}

void BattleFrameMapDrawer::applyPolygon()
{
    if(_polygonPoints.count() >= 3 && _scene)
    {
        LayerFow* layer = dynamic_cast<LayerFow*>(_scene->getNearest(_scene->getSelectedLayer(), DMHelper::LayerType_Fow));
        if(layer)
        {
            QPolygon adjustedPolygon = _polygonPoints;
            adjustedPolygon.translate(-layer->getPosition());
            UndoFowPolygon* undoPolygon = new UndoFowPolygon(layer, MapEditPolygon(adjustedPolygon, _erase, false));
            layer->getUndoStack()->push(undoPolygon);
            emit dirty();
        }
    }
    _polygonPoints.clear();
    emit polygonCancelled();
}

void BattleFrameMapDrawer::cancelPolygon()
{
    _polygonPoints.clear();
    emit polygonCancelled();
}

void BattleFrameMapDrawer::cancelSelect()
{
    _selectActive = false;
    emit selectRectCancelled();
}

void BattleFrameMapDrawer::createCursor()
{
    if(_brushMode == DMHelper::BrushType_Polygon)
    {
        _cursor = QCursor(QPixmap(":/img/data/crosshair.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        emit cursorChanged(_cursor);
        return;
    }

    if(_brushMode == DMHelper::BrushType_Select)
    {
        _cursor = QCursor(QPixmap(":/img/data/icon_selectcursor.png").scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        emit cursorChanged(_cursor);
        return;
    }

    int cursorSize = _gridScale * _zoomScale * _size / 5;
    QPixmap cursorPixmap(QSize(cursorSize, cursorSize));
    cursorPixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&cursorPixmap);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QBrush(Qt::black), 4));
        if(_brushMode == DMHelper::BrushType_Circle)
            painter.drawEllipse(0, 0, cursorSize, cursorSize);
        else
            painter.drawRect(0, 0, cursorSize, cursorSize);
    painter.end();

    _cursor = QCursor(cursorPixmap);
    emit cursorChanged(_cursor);
}
