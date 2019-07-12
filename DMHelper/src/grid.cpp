#include "grid.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QGraphicsScene>

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
    {
        _gridShape = gridShape;
    }
}

QRect Grid::getGridShape() const
{
    return _gridShape;
}

void Grid::setGridVisible(bool gridVisible)
{
    for(QGraphicsItem* item : _grid)
    {
        item->setVisible(gridVisible);
    }
}

void Grid::clear()
{
    qDeleteAll(_grid);
    _grid.clear();
}

void Grid::rebuildGrid(BattleDialogModel& model)
{
    clear();

    if(model.getGridOn() == false)
        return;

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
