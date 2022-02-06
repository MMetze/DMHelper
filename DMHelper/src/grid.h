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

    void rebuildGrid(BattleDialogModel& model, QPainter* painter = nullptr);

private:

    void rebuildGrid_Square(BattleDialogModel& model, QPainter* painter = nullptr);
    void rebuildGrid_Hex(BattleDialogModel& model, QPainter* painter = nullptr);
    void rebuildGrid_Isosquare(BattleDialogModel& model, QPainter* painter = nullptr);
    void rebuildGrid_Isohex(BattleDialogModel& model, QPainter* painter = nullptr);

    int computeOutCode(int x, int y);
    void createLine(int x1, int y1, int x2, int y2, QPainter* painter);

    QRect _gridShape;
    QList<QGraphicsItem*> _grid;
};

#endif // GRID_H
