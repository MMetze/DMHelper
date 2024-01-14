#ifndef GRID_H
#define GRID_H

#include "dmconstants.h"
#include <QGraphicsItemGroup>
#include <QPoint>
#include <QPen>

class QGraphicsScene;
class GridConfig;

class Grid_LineInterface
{
public:
    virtual void addLine(int x0, int y0, int x1, int y1, int zOrder) = 0;
};

class Grid : public QGraphicsItemGroup, public Grid_LineInterface
{
public:
    enum GridType
    {
        GridType_Square = 0,
        GridType_Hex,
        GridType_Isosquare,
        GridType_Isohex
    };

    Grid(QGraphicsScene* graphicsScene, const QRect& gridShape);
    virtual ~Grid();

    virtual void setGridShape(const QRect& gridShape);
    virtual void setGridSize(const QSize& gridSize);
    virtual void setGridPosition(const QPoint& position);
    QRect getGridShape() const;
    virtual void setGridZValue(int zOrder);
    virtual void setGridVisible(bool gridVisible);
    virtual void setGridOpacity(qreal gridOpacity);
    virtual void clear();

    virtual void addLine(int x0, int y0, int x1, int y1, int zOrder) override;

    void rebuildGrid(GridConfig& config, int zOrder = DMHelper::BattleDialog_Z_Grid, Grid_LineInterface* grid = nullptr);

private:

    void rebuildGrid_Square(GridConfig& config, int zOrder, Grid_LineInterface* grid);
    void rebuildGrid_Hex(GridConfig& config, int zOrder, Grid_LineInterface* grid);
    void rebuildGrid_Isosquare(GridConfig& config, int zOrder, Grid_LineInterface* grid);
    void rebuildGrid_Isohex(GridConfig& config, int zOrder, Grid_LineInterface* grid);

    int computeOutCode(int x, int y);
    void createLine(int x1, int y1, int x2, int y2, int zOrder, Grid_LineInterface* grid);

    QRect _gridShape;
    QList<QGraphicsItem*> _grid;
    QPen _localPen;
};

#endif // GRID_H
