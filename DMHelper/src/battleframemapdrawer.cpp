#include "battleframemapdrawer.h"
#include "undofowpath.h"
#include "undofowfill.h"
#include "undofowshape.h"
#include "map.h"
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>

BattleFrameMapDrawer::BattleFrameMapDrawer(QObject *parent) :
    QObject(parent),
    _mouseDown(false),
    _mouseDownPos(),
    _undoPath(nullptr),
    _map(nullptr),
    _fow(nullptr),
    _glFow(nullptr),
    _cursor(),
    _gridScale(10),
    _viewWidth(10),
    _size(10),
    _erase(true),
    _smooth(true),
    _brushMode(DMHelper::BrushType_Circle)

{
    createCursor();
}

void BattleFrameMapDrawer::setMap(Map* map, QPixmap* fow, QImage* glFow)
{
    _map = map;
    _fow = fow;
    _glFow = glFow;

    if((_map) && (_fow) && (_glFow))
    {
        emit fowEdited(*_fow);
        emit fowChanged(*_glFow);
    }
}

Map* BattleFrameMapDrawer::getMap() const
{
    return _map;
}

const QCursor& BattleFrameMapDrawer::getCursor() const
{
    return _cursor;
}

void BattleFrameMapDrawer::handleMouseDown(const QPointF& pos)
{
    if((!_map) || (!_fow) || (!_glFow))
        return;

    _mouseDownPos = pos;
    _mouseDown = true;

    // TODO: Layers
    // Math says divide by 10: radius of 5 to adjust scale to "one square"
    LayerFow* layer = dynamic_cast<LayerFow*>(_map->getLayerScene().getPriority(DMHelper::LayerType_Fow));
    if(layer)
    {
        _undoPath = new UndoFowPath(layer, MapDrawPath(_gridScale * _size / 10, _brushMode, _erase, _smooth, pos.toPoint()));
        layer->getUndoStack()->push(_undoPath);
        //_map->paintFoWPoint(pos.toPoint(), _undoPath->mapDrawPath(), _fow, true);
        //_map->paintFoWPoint(pos.toPoint(), _undoPath->mapDrawPath(), _glFow, false);
        emit fowEdited(*_fow);
    }

    //_undoPath = new UndoFowPath(_map, MapDrawPath(_gridScale * _size / 10, _brushMode, _erase, _smooth, pos.toPoint()));
    //_map->getUndoStack()->push(_undoPath);
    //_map->paintFoWPoint(pos.toPoint(), _undoPath->mapDrawPath(), _fow, true);
    //_map->paintFoWPoint(pos.toPoint(), _undoPath->mapDrawPath(), _glFow, false);
    //emit fowEdited(*_fow);
}

void BattleFrameMapDrawer::handleMouseMoved(const QPointF& pos)
{
    if((!_map) || (!_undoPath) || (!_fow) || (!_glFow))
        return;

    _undoPath->addPoint(pos.toPoint());
    // TODO: Layers
    //_map->paintFoWPoint(pos.toPoint(), _undoPath->mapDrawPath(), _fow, true);
    //_map->paintFoWPoint(pos.toPoint(), _undoPath->mapDrawPath(), _glFow, false);
    emit fowEdited(*_fow);
}

void BattleFrameMapDrawer::handleMouseUp(const QPointF& pos)
{
    Q_UNUSED(pos);
    if(_glFow)
        emit fowChanged(*_glFow);
    endPath();
}

void BattleFrameMapDrawer::drawRect(const QRect& rect)
{
    if((!_map) || (!_fow) || (!_glFow))
        return;

    // TODO: Layers
    /*
    // Changed to ignore smoothing on an area
    UndoFowShape* undoShape = new UndoFowShape(_map, MapEditShape(rect, _erase, false));
    _map->getUndoStack()->push(undoShape);
    //_map->paintFoWRect(rect, undoShape->mapEditShape(), _fow, true);
    //_map->paintFoWRect(rect, undoShape->mapEditShape(), _glFow, false);
    emit fowEdited(*_fow);
    emit fowChanged(*_glFow);
    endPath();
    */
}

void BattleFrameMapDrawer::setSize(int size)
{
    if(_size == size)
        return;

    _size = size;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::setScale(int gridScale, int viewWidth)
{
    if((_gridScale == gridScale) && (_viewWidth == viewWidth))
        return;

    _gridScale = gridScale;
    _viewWidth = viewWidth;
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
    if((!_map) || (!_fow) || (!_glFow))
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Fill FoW"), QString("Are you sure you would like to fill the entire Fog of War?")) == QMessageBox::No)
        return;

    // TODO: Layers
    /*
    UndoFowFill* undoFill = new UndoFowFill(_map, MapEditFill(QColor(0, 0, 0, 255)));
    _map->getUndoStack()->push(undoFill);
    //_map->fillFoW(QColor(0, 0, 0, 128), _fow);
    //_map->fillFoW(QColor(0, 0, 0, 255), _glFow);
    endPath();
    emit fowEdited(*_fow);
    emit fowChanged(*_glFow);
    */
}

void BattleFrameMapDrawer::clearFoW()
{
    if((!_map) || (!_fow) || (!_glFow))
        return;

    if(QMessageBox::question(nullptr, QString("Confirm Clear FoW"), QString("Are you sure you would like to clear the entire Fog of War?")) == QMessageBox::No)
        return;

    // TODO: Layers
    /*
    UndoFowFill* undoFill = new UndoFowFill(_map, MapEditFill(QColor(0, 0, 0, 0)));
    _map->getUndoStack()->push(undoFill);
    //_map->fillFoW(QColor(0, 0, 0, 0), _fow);
    //_map->fillFoW(QColor(0, 0, 0, 0), _glFow);
    endPath();
    emit fowEdited(*_fow);
    emit fowChanged(*_glFow);
    */
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

    _brushMode = brushMode;
    endPath();
    createCursor();
}

void BattleFrameMapDrawer::endPath()
{
    _undoPath = nullptr;
    _mouseDown = false;
}

void BattleFrameMapDrawer::createCursor()
{
    int cursorSize = _viewWidth * _size / 5;
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
