#include "grid.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QGraphicsScene>
#include <QtMath>
#include <QDebug>

Grid::Grid(QGraphicsScene& graphicsScene, const QRect& gridShape) :
    QGraphicsItemGroup(),
    _gridShape(gridShape),
    _grid()
{
    graphicsScene.addItem(this);
}

Grid::~Grid()
{
}

void Grid::setGridShape(const QRect& gridShape)
{
    if(_gridShape != gridShape)
        _gridShape = gridShape;
}

QRect Grid::getGridShape() const
{
    return _gridShape;
}

void Grid::setGridVisible(bool gridVisible)
{
    for(QGraphicsItem* item : _grid)
        item->setVisible(gridVisible);
}

void Grid::clear()
{
    qDeleteAll(_grid);
    _grid.clear();
}

void Grid::rebuildGrid(BattleDialogModel& model, int gridType)
{
    clear();

    if(model.getGridOn() == false)
        return;

    switch(gridType)
    {
        case GridType_Square:
            rebuildGrid_Square(model);
            break;
        case GridType_Hex:
            rebuildGrid_Hex(model);
            break;
        case GridType_Isosquare:
            rebuildGrid_Isosquare(model);
            break;
        case GridType_Isohex:
            rebuildGrid_Isohex(model);
            break;
        default:
            qDebug() << "[Grid] ERROR: Invalid grid type requested (" << gridType << "). No grid drawn";
            break;
    }
}

void Grid::rebuildGrid_Square(BattleDialogModel& model)
{
    int xOffset = model.getGridScale() * model.getGridOffsetX() / 100;
    int yOffset = model.getGridScale() * model.getGridOffsetY() / 100;
    int xCount = (_gridShape.width() - xOffset) / model.getGridScale();
    int yCount = (_gridShape.height() - yOffset) / model.getGridScale();

    // Set an outline
    QGraphicsItem* newItem = scene()->addRect(_gridShape, QPen(Qt::SolidLine));
    newItem->setZValue(DMHelper::BattleDialog_Z_Grid);
    _grid.append(newItem);

    for(int x = 0; x <= xCount; ++x)
    {
        QGraphicsItem* newLineItem = scene()->addLine((x*model.getGridScale()) + xOffset, 0, (x*model.getGridScale()) + xOffset, _gridShape.height());
        if(newLineItem)
        {
            newLineItem->setZValue(DMHelper::BattleDialog_Z_Grid);
            _grid.append(newLineItem);
        }
    }

    for(int y = 0; y <= yCount; ++y)
    {
        QGraphicsItem* newLineItem = scene()->addLine(0, (y*model.getGridScale()) + yOffset, _gridShape.width(), (y*model.getGridScale()) + yOffset);
        if(newLineItem)
        {
            newLineItem->setZValue(DMHelper::BattleDialog_Z_Grid);
            _grid.append(newLineItem);
        }
    }
}

void Grid::rebuildGrid_Hex(BattleDialogModel& model)
{
    Q_UNUSED(model);
}

void Grid::rebuildGrid_Isosquare(BattleDialogModel& model)
{
    int isoScale = model.getGridScale() * 2;
    int xOffset = isoScale * model.getGridOffsetX() / 100;
    int yOffset = isoScale * model.getGridOffsetY() / 100;
    int xCount = (_gridShape.width() - xOffset) / isoScale;
    int yCount = (_gridShape.height() - yOffset) / (isoScale * qTan(qDegreesToRadians(30.0)));
    int xDelta = static_cast<int>(static_cast<qreal>(_gridShape.height()) / qTan(qDegreesToRadians(30.0)));
    int xStart = xDelta / isoScale;

    if(xDelta <= 0)
    {
        qDebug() << "[Grid] ERROR: Isometric grid requested with invalid delta: " << xDelta << ". No grid drawn";
        return;
    }

    // Set an outline
    QGraphicsItem* newItem = scene()->addRect(_gridShape, QPen(Qt::SolidLine));
    newItem->setZValue(DMHelper::BattleDialog_Z_Grid);
    _grid.append(newItem);

    // Lines from the top to the right
    for(int x = -xStart; x <= xCount; ++x)
    {
        int left = (x*isoScale) + xOffset;
        int right = (x*isoScale) + xOffset + xDelta;
        int top = 0;
        int bottom = _gridShape.height();
        if(left < 0)
        {
            top = bottom * -left / xDelta;
            left = 0;
        }
        if(right > _gridShape.width())
        {
            bottom = bottom * (_gridShape.width() - (x*isoScale) - xOffset) / xDelta;
            right = _gridShape.width();
        }
        QGraphicsItem* newLineItem = scene()->addLine(left, top, right, bottom);
        if(newLineItem)
        {
            newLineItem->setZValue(DMHelper::BattleDialog_Z_Grid);
            _grid.append(newLineItem);
        }
    }

    // Lines from the top to the right
    for(int x = 0; x <= xCount + xStart; ++x)
    {
        int left = (x*isoScale) + xOffset - xDelta;
        int right = (x*isoScale) + xOffset;
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
        QGraphicsItem* newLineItem = scene()->addLine(right, top, left, bottom);
        if(newLineItem)
        {
            newLineItem->setZValue(DMHelper::BattleDialog_Z_Grid);
            _grid.append(newLineItem);
        }
    }

    // Lines from the left to the right
    /*
    for(int y = 0; y <= yCount; ++y)
    {
        int left = 0;
        int top = (y*isoScale) + yOffset;
        int right = (x*isoScale) + xOffset + xDelta;
        int bottom = _gridShape.height();
        if(right > _gridShape.width())
        {
            bottom = bottom * (_gridShape.width() - left) / xDelta;
            right = _gridShape.width();
        }
        QGraphicsItem* newLineItem = scene()->addLine(left, top, right, bottom);
        if(newLineItem)
        {
            newLineItem->setZValue(DMHelper::BattleDialog_Z_Grid);
            _grid.append(newLineItem);
        }
    }
    */

    /*
    for(int x = 0; x <= xCount; ++x)
    {
        QGraphicsItem* newLineItem = scene()->addLine((x*isoScale) + xOffset, 0, (x*isoScale) + xOffset - xDelta, _gridShape.height());
        if(newLineItem)
        {
            newLineItem->setZValue(DMHelper::BattleDialog_Z_Grid);
            _grid.append(newLineItem);
        }
    }
    */

    /*
    for(int y = 0; y <= yCount; ++y)
    {
        QGraphicsItem* newLineItem = scene()->addLine(0, (y*model.getGridScale()) + yOffset, _gridShape.width(), (y*model.getGridScale()) + yOffset);
        if(newLineItem)
        {
            newLineItem->setZValue(DMHelper::BattleDialog_Z_Grid);
            _grid.append(newLineItem);
        }
    }
    */
}

void Grid::rebuildGrid_Isohex(BattleDialogModel& model)
{
    Q_UNUSED(model);
}

