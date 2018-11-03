#ifndef GRID_H
#define GRID_H

#include <QGraphicsItemGroup>
#include <QPoint>

class QGraphicsScene;
class BattleDialogModel;

class Grid : public QGraphicsItemGroup
{
public:
    Grid(QGraphicsScene& graphicsScene, const QRect& gridShape);
    virtual ~Grid();

    virtual void setGridShape(const QRect& gridShape);
    QRect getGridShape() const;
    virtual void setGridVisible(bool gridVisible);
    virtual void clear();

    void rebuildGrid(BattleDialogModel& model);

private:

    QRect _gridShape;
    QList<QGraphicsItem*> _grid;
};

#endif // GRID_H
