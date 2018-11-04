#ifndef UNSELECTEDPIXMAP_H
#define UNSELECTEDPIXMAP_H

#include <QGraphicsPixmapItem>

class BattleDialogModelCombatant;

class UnselectedPixmap : public QGraphicsPixmapItem
{
public:
    UnselectedPixmap(BattleDialogModelCombatant* combatant = 0);
    UnselectedPixmap(const QPixmap &pixmap, BattleDialogModelCombatant* combatant);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    BattleDialogModelCombatant* _combatant;
};

#endif // UNSELECTEDPIXMAP_H
