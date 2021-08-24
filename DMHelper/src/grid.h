#ifndef GRID_H
#define GRID_H

#include <QGraphicsItemGroup>
#include <QPoint>

class QGraphicsScene;
class BattleDialogModel;

class Grid : public QGraphicsItemGroup
{
public:
    enum GridType
    {
        GridType_Square = 0,
        GridType_Hex,
        GridType_Isosquare,
        GridType_Isohex
    };

    Grid(QGraphicsScene& graphicsScene, const QRect& gridShape);
    virtual ~Grid();

    virtual void setGridShape(const QRect& gridShape);
    QRect getGridShape() const;
    virtual void setGridVisible(bool gridVisible);
    virtual void clear();

    void rebuildGrid(BattleDialogModel& model, int gridType);

private:

    void rebuildGrid_Square(BattleDialogModel& model);
    void rebuildGrid_Hex(BattleDialogModel& model);
    void rebuildGrid_Isosquare(BattleDialogModel& model);
    void rebuildGrid_Isohex(BattleDialogModel& model);

    QRect _gridShape;
    QList<QGraphicsItem*> _grid;
};

#endif // GRID_H
