#ifndef BATTLETOKENHEALTHBAR_H
#define BATTLETOKENHEALTHBAR_H

#include <QGraphicsObject>

class BattleDialogModelCombatant;

class BattleTokenHealthBar : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit BattleTokenHealthBar(BattleDialogModelCombatant* combatant, QGraphicsItem* parent = nullptr);
    virtual ~BattleTokenHealthBar() override;

    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    BattleDialogModelCombatant* _combatant;
};

#endif // BATTLETOKENHEALTHBAR_H
