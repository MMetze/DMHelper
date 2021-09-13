#ifndef UNSELECTEDPIXMAP_H
#define UNSELECTEDPIXMAP_H

#include <QGraphicsPixmapItem>

class BattleDialogModelCombatant;

class UnselectedPixmap : public QGraphicsPixmapItem
{
public:
    explicit UnselectedPixmap(BattleDialogModelCombatant* combatant = nullptr, QGraphicsItem *parent = nullptr);
    UnselectedPixmap(const QPixmap &pixmap, BattleDialogModelCombatant* combatant, QGraphicsItem *parent = nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
    void setDraw(bool draw);

    void setHover(bool hover);
    BattleDialogModelCombatant* getCombatant();

protected:
//    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
//    virtual bool sceneEvent(QEvent *event);

    void selectionChanged();

private:
    BattleDialogModelCombatant* _combatant;
    bool _draw;
};

#endif // UNSELECTEDPIXMAP_H
