#ifndef UNSELECTEDPOLYGON_H
#define UNSELECTEDPOLYGON_H

#include <QGraphicsPolygonItem>

class BattleDialogModelObject;

class UnselectedPolygon : public QGraphicsPolygonItem
{
public:
    UnselectedPolygon(BattleDialogModelObject* object, const QPolygonF &polygon, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    BattleDialogModelObject* _object;

};

#endif // UNSELECTEDPOLYGON_H
