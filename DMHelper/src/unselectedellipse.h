#ifndef UNSELECTEDELLIPSE_H
#define UNSELECTEDELLIPSE_H

#include <QGraphicsEllipseItem>

class BattleDialogModelObject;

class UnselectedEllipse : public QGraphicsEllipseItem
{
public:
    UnselectedEllipse(BattleDialogModelObject* object, qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    BattleDialogModelObject* _object;
};

#endif // UNSELECTEDELLIPSE_H
