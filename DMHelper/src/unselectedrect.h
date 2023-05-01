#ifndef UNSELECTEDRECT_H
#define UNSELECTEDRECT_H

#include <QGraphicsRectItem>

class BattleDialogModelObject;

class UnselectedRect : public QGraphicsRectItem
{
public:
    UnselectedRect(BattleDialogModelObject* object, qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    BattleDialogModelObject* _object;

};

#endif // UNSELECTEDRECT_H
