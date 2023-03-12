#include "grid.h"
#include "gridconfig.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>
#include <QDebug>

Grid::Grid(QGraphicsScene* graphicsScene, const QRect& gridShape) :
    QGraphicsItemGroup(),
    _gridShape(gridShape),
    _grid(),
    _localPen()
{
    if(graphicsScene)
        graphicsScene->addItem(this);
}

Grid::~Grid()
{
}

void Grid::setGridShape(const QRect& gridShape)
{
    if(_gridShape != gridShape)
        _gridShape = gridShape;
}

void Grid::setGridSize(const QSize& gridSize)
{
    if(_gridShape.size() != gridSize)
        _gridShape.setSize(gridSize);
}

void Grid::setGridPosition(const QPoint& position)
{
    if(_gridShape.topLeft() == position)
        return;

    qreal dx = position.x() - _gridShape.left();
    qreal dy = position.y() - _gridShape.top();

    for(int i = 0; i < _grid.count(); ++i)
    {
        if(_grid[i])
            _grid[i]->moveBy(dx, dy);
    }

    _gridShape.moveTopLeft(position);
}

QRect Grid::getGridShape() const
{
    return _gridShape;
}

void Grid::setGridZValue(int zOrder)
{
    for(int i = 0; i < _grid.count(); ++i)
    {
        if(_grid[i])
            _grid[i]->setZValue(zOrder);
    }
}

void Grid::setGridVisible(bool gridVisible)
{
    for(int i = 0; i < _grid.count(); ++i)
    {
        if(_grid[i])
            _grid[i]->setVisible(gridVisible);
    }
}

void Grid::setGridOpacity(bool gridOpacity)
{
    for(int i = 0; i < _grid.count(); ++i)
    {
        if(_grid[i])
            _grid[i]->setOpacity(gridOpacity);
    }
}

void Grid::clear()
{
    qDeleteAll(_grid);
    _grid.clear();
}

void Grid::addLine(int x0, int y0, int x1, int y1, int zOrder)
{
    if(!scene())
        return;

    QGraphicsItem* newLineItem = scene()->addLine(_gridShape.left() + x0,
                                                  _gridShape.top() + y0,
                                                  _gridShape.left() + x1,
                                                  _gridShape.top() + y1,
                                                  _localPen);

    if(newLineItem)
    {
        newLineItem->setZValue(zOrder);
        _grid.append(newLineItem);
    }
}

void Grid::rebuildGrid(GridConfig& config, int zOrder, Grid_LineInterface* grid)
{
    if(!grid)
    {
        clear();
        _localPen = config.getGridPen();
        grid = this;
    }

    if(config.getGridOn() == false)
        return;

    switch(config.getGridType())
    {
        case GridType_Square:
            rebuildGrid_Square(config, zOrder, grid);
            break;
        case GridType_Hex:
            rebuildGrid_Hex(config, zOrder, grid);
            break;
        case GridType_Isosquare:
            rebuildGrid_Isosquare(config, zOrder, grid);
            break;
        case GridType_Isohex:
            rebuildGrid_Isohex(config, zOrder, grid);
            break;
        default:
            qDebug() << "[Grid] ERROR: Invalid grid type requested (" << config.getGridType() << "). No grid drawn";
            break;
    }
}

void Grid::rebuildGrid_Square(GridConfig& config, int zOrder, Grid_LineInterface* grid)
{
    int xOffset = config.getGridScale() * config.getGridOffsetX() / 100;
    int yOffset = config.getGridScale() * config.getGridOffsetY() / 100;
    int xCount = (_gridShape.width() - xOffset) / config.getGridScale();
    int yCount = (_gridShape.height() - yOffset) / config.getGridScale();

    // Set an outline
    //QGraphicsItem* newItem = scene()->addRect(_gridShape, QPen(Qt::SolidLine));
    //newItem->setZValue(DMHelper::BattleDialog_Z_Grid);
    //_grid.append(newItem);

    for(int x = 0; x <= xCount; ++x)
    {
        createLine((x*config.getGridScale()) + xOffset, 0, (x*config.getGridScale()) + xOffset, _gridShape.height(), zOrder, grid);
    }

    for(int y = 0; y <= yCount; ++y)
    {
        createLine(0, (y*config.getGridScale()) + yOffset, _gridShape.width(), (y*config.getGridScale()) + yOffset, zOrder, grid);
    }
}

void Grid::rebuildGrid_Hex(GridConfig& config, int zOrder, Grid_LineInterface* grid)
{
    int hexScale = config.getGridScale();
    int hexScaleShort = config.getGridScale() * 0.5;
    int hexScaleLong = config.getGridScale() * 0.866;
    int hexScaleStep = 3 * hexScale;
    int xOffset = hexScale * config.getGridOffsetX() / 100;
    int yOffset = hexScale * config.getGridOffsetY() / 100;
    //int xCount = (_gridShape.width() - xOffset) / hexScale;
    int xCount = (_gridShape.width() - xOffset) / hexScaleStep;
    int yCount = (_gridShape.height() - yOffset) / hexScale;

    // Set an outline
    //QGraphicsItem* newItem = scene()->addRect(_gridShape, QPen(Qt::SolidLine));
    //newItem->setZValue(DMHelper::BattleDialog_Z_Grid);
    //_grid.append(newItem);

    for(int y = 0; y <= yCount; ++y)
    {
        for(int x = 0; x <= xCount; ++x)
        {
            createLine((x*hexScaleStep) + xOffset, (y*hexScaleLong*2) + yOffset,
                       (x*hexScaleStep) + xOffset + hexScale, (y*hexScaleLong*2) + yOffset, zOrder, grid);
            createLine((x*hexScaleStep) + xOffset + hexScale, (y*hexScaleLong*2) + yOffset,
                       (x*hexScaleStep) + xOffset + hexScale + hexScaleShort, (y*hexScaleLong*2) + yOffset + hexScaleLong, zOrder, grid);
            createLine((x*hexScaleStep) + xOffset + hexScale, (y*hexScaleLong*2) + yOffset,
                       (x*hexScaleStep) + xOffset + hexScale + hexScaleShort, (y*hexScaleLong*2) + yOffset - hexScaleLong, zOrder, grid);
            createLine((x*hexScaleStep) + xOffset + hexScale + hexScaleShort, (y*hexScaleLong*2) + yOffset + hexScaleLong,
                       (x*hexScaleStep) + xOffset + (2*hexScale) + hexScaleShort, (y*hexScaleLong*2) + yOffset + hexScaleLong, zOrder, grid);
            createLine((x*hexScaleStep) + xOffset + (2*hexScale) + hexScaleShort, (y*hexScaleLong*2) + yOffset + hexScaleLong,
                       (x*hexScaleStep) + xOffset + (2*hexScale) + (2*hexScaleShort), (y*hexScaleLong*2) + yOffset, zOrder, grid);
            createLine((x*hexScaleStep) + xOffset + (2*hexScale) + hexScaleShort, (y*hexScaleLong*2) + yOffset + hexScaleLong,
                       (x*hexScaleStep) + xOffset + (2*hexScale) + (2*hexScaleShort), (y*hexScaleLong*2) + yOffset + (hexScaleLong*2), zOrder, grid);
        }
    }
}

void Grid::rebuildGrid_Isosquare(GridConfig& config, int zOrder, Grid_LineInterface* grid)
{
    int isoScale = config.getGridScale() * 3;
    int xOffset = isoScale * config.getGridOffsetX() / 100;
    int yOffset = isoScale * config.getGridOffsetY() / 100;
    int totalOffset = xOffset + yOffset;
    int xCount = (_gridShape.width() - totalOffset) / isoScale;
    //    int xDelta = static_cast<int>(static_cast<qreal>(_gridShape.height()) / qTan(qDegreesToRadians(30.0)));
    int xDelta = static_cast<int>(static_cast<qreal>(_gridShape.height()) / qTan(qDegreesToRadians(static_cast<qreal>(config.getGridAngle()) / 2.0)));
    int xStart = xDelta / isoScale;

    if(xDelta <= 0)
    {
        qDebug() << "[Grid] ERROR: Isometric grid requested with invalid delta: " << xDelta << ". No grid drawn";
        return;
    }

    // Set an outline
    //QGraphicsItem* newItem = scene()->addRect(_gridShape, QPen(Qt::SolidLine));
    //newItem->setZValue(DMHelper::BattleDialog_Z_Grid);
    //_grid.append(newItem);

    // Lines from the top to the right
    for(int x = -xStart; x <= xCount; ++x)
    {
        int left = (x*isoScale) + totalOffset;
        int right = (x*isoScale) + totalOffset + xDelta;
        int top = 0;
        int bottom = _gridShape.height();
        if(left < 0)
        {
            top = bottom * -left / xDelta;
            left = 0;
        }
        if(right > _gridShape.width())
        {
            bottom = bottom * (_gridShape.width() - (x*isoScale) - totalOffset) / xDelta;
            right = _gridShape.width();
        }

        createLine(left, top, right, bottom, zOrder, grid);
    }

    // Lines from the top to the right
    for(int x = 0; x <= xCount + xStart; ++x)
    {
        int left = (x*isoScale) + totalOffset - xDelta;
        int right = (x*isoScale) + totalOffset;
        int top = 0;
        int bottom = _gridShape.height();
        if(left < 0)
        {
            bottom = bottom * (xDelta + left) / xDelta;
            left = 0;
        }
        if(right > _gridShape.width())
        {
            top = _gridShape.height() * (right - _gridShape.width()) / xDelta;
            right = _gridShape.width();
        }

        createLine(right, top, left, bottom, zOrder, grid);
    }
}

void Grid::rebuildGrid_Isohex(GridConfig& config, int zOrder, Grid_LineInterface* grid)
{
    qreal isoAngle = qCos(qDegreesToRadians(static_cast<qreal>(config.getGridAngle())));
    int hexScale = static_cast<int>(static_cast<qreal>(config.getGridScale()) * isoAngle);
    int hexScaleShort = static_cast<int>(static_cast<qreal>(config.getGridScale()) * 0.5 * isoAngle);
    int hexScaleLong = static_cast<int>(static_cast<qreal>(config.getGridScale()) * 0.866);
    int hexScaleStep = 3 * hexScale;
    int xOffset = config.getGridScale() * config.getGridOffsetX() / 100;
    int yOffset = hexScale * config.getGridOffsetY() / 100;
    int xCount = (_gridShape.width() - xOffset) / (hexScaleLong * 2);
    int yCount = (_gridShape.height() - yOffset) / hexScaleStep;

    // Set an outline
    //QGraphicsItem* newItem = scene()->addRect(_gridShape, QPen(Qt::SolidLine));
    //newItem->setZValue(DMHelper::BattleDialog_Z_Grid);
    //_grid.append(newItem);

    for(int x = 0; x <= xCount; ++x)
    {
        for(int y = 0; y <= yCount; ++y)
        {
            createLine((x*hexScaleLong*2) + xOffset,                    (y*hexScaleStep) + yOffset,
                       (x*hexScaleLong*2) + xOffset,                    (y*hexScaleStep) + yOffset + hexScale, zOrder, grid);

            createLine((x*hexScaleLong*2) + xOffset,                    (y*hexScaleStep) + yOffset + hexScale,
                       (x*hexScaleLong*2) + xOffset + hexScaleLong,     (y*hexScaleStep) + yOffset + hexScale + hexScaleShort, zOrder, grid);
            createLine((x*hexScaleLong*2) + xOffset,                    (y*hexScaleStep) + yOffset + hexScale,
                       (x*hexScaleLong*2) + xOffset - hexScaleLong,     (y*hexScaleStep) + yOffset + hexScale + hexScaleShort, zOrder, grid);

            createLine((x*hexScaleLong*2) + xOffset + hexScaleLong,     (y*hexScaleStep) + yOffset + hexScale + hexScaleShort,
                       (x*hexScaleLong*2) + xOffset + hexScaleLong,     (y*hexScaleStep) + yOffset + (2*hexScale) + hexScaleShort, zOrder, grid);

            createLine((x*hexScaleLong*2) + xOffset + hexScaleLong,     (y*hexScaleStep) + yOffset + (2*hexScale) + hexScaleShort,
                       (x*hexScaleLong*2) + xOffset,                    (y*hexScaleStep) + yOffset + (2*hexScale) + (hexScaleShort*2), zOrder, grid);
            createLine((x*hexScaleLong*2) + xOffset - (hexScaleLong), (y*hexScaleStep) + yOffset + (2*hexScale) + hexScaleShort,
                       (x*hexScaleLong*2) + xOffset,                    (y*hexScaleStep) + yOffset + (2*hexScale) + (hexScaleShort*2), zOrder, grid);
        }
    }
}

// For clipping the lines, use the CohenSutherland implementation
// Source: https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm

const int GRID_INSIDE = 0; // 0000
const int GRID_LEFT = 1;   // 0001
const int GRID_RIGHT = 2;  // 0010
const int GRID_BOTTOM = 4; // 0100
const int GRID_TOP = 8;    // 1000

// Compute the bit code for a point (x, y) using the clip bounded diagonally by (xmin, ymin), and (xmax, ymax)
int Grid::computeOutCode(int x, int y)
{
    int code = GRID_INSIDE;             // initialised as being inside of [[clip window]]

    if (x < 0)                          // to the left of clip window
        code |= GRID_LEFT;
    else if (x > _gridShape.width())    // to the right of clip window
        code |= GRID_RIGHT;
    if (y < 0)                          // below the clip window
        code |= GRID_BOTTOM;
    else if (y > _gridShape.height())   // above the clip window
        code |= GRID_TOP;

    return code;
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
// diagonal from (xmin, ymin) to (xmax, ymax).

void Grid::createLine(int x0, int y0, int x1, int y1, int zOrder, Grid_LineInterface* grid)
{
    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    int outcode0 = computeOutCode(x0, y0);
    int outcode1 = computeOutCode(x1, y1);

    // bitwise OR is 0: both points inside window; trivially accept and exit loop
    while(outcode0 | outcode1)
    {
        // bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
        // or BOTTOM), so both must be outside window; exit loop (accept is false)
        if(outcode0 & outcode1)
            return;

        // failed both trivial tests, so calculate the line segment to clip from an outside point to an intersection with clip edge
        int x, y;

        // At least one endpoint is outside the clip rectangle; pick it.
        int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

        // Now find the intersection point, use formulas:
        //   slope = (y1 - y0) / (x1 - x0)
        //   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
        //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
        // No need to worry about divide-by-zero because, in each case, the outcode bit being tested guarantees the denominator is non-zero
        if (outcodeOut & GRID_TOP) {           // point is above the clip window
            x = static_cast<int>(static_cast<qreal>(x0) + static_cast<qreal>(x1 - x0) * static_cast<qreal>(_gridShape.height() - y0) / static_cast<qreal>(y1 - y0));
            y = _gridShape.height();
        } else if (outcodeOut & GRID_BOTTOM) { // point is below the clip window
            x = static_cast<int>(static_cast<qreal>(x0) + static_cast<qreal>(x1 - x0) * static_cast<qreal>(-y0) / static_cast<qreal>(y1 - y0));
            y = 0;
        } else if (outcodeOut & GRID_RIGHT) {  // point is to the right of clip window
            y = static_cast<int>(static_cast<qreal>(y0) + static_cast<qreal>(y1 - y0) * static_cast<qreal>(_gridShape.width() - x0) / static_cast<qreal>(x1 - x0));
            x = _gridShape.width();
        } else if (outcodeOut & GRID_LEFT) {   // point is to the left of clip window
            y = static_cast<int>(static_cast<qreal>(y0) + static_cast<qreal>(y1 - y0) * static_cast<qreal>(-x0) / static_cast<qreal>(x1 - x0));
            x = 0;
        }

        // Now we move outside point to intersection point to clip and get ready for next pass.
        if (outcodeOut == outcode0) {
            x0 = x;
            y0 = y;
            outcode0 = computeOutCode(x0, y0);
        } else {
            x1 = x;
            y1 = y;
            outcode1 = computeOutCode(x1, y1);
        }
    }

    grid->addLine(x0, y0, x1, y1, zOrder);
}
