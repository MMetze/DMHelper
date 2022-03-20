#ifndef BATTLECOMBATANTFRAME_H
#define BATTLECOMBATANTFRAME_H

#include <QFrame>
#include <QGridLayout>
#include "combatant.h"

namespace Ui {
class BattleCombatantFrame;
}

class BattleDialogModelCombatant;

class BattleCombatantFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BattleCombatantFrame(QWidget *parent = nullptr);
    explicit BattleCombatantFrame(BattleDialogModelCombatant* combatant, QWidget *parent = nullptr);
    ~BattleCombatantFrame();

    void setCombatant(BattleDialogModelCombatant* combatant);
    BattleDialogModelCombatant* getCombatant() const;

signals:
    void conditionsChanged(BattleDialogModelCombatant* combatant);

protected slots:
    void editConditions();
    void removeConditions();
    void readCombatant();
    void clearCombatant();
    void updateLayout();
    void clearGrid();
    void addCondition(Combatant::Condition condition);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Ui::BattleCombatantFrame *ui;

    BattleDialogModelCombatant* _combatant;
    QGridLayout* _conditionGrid;
};

#endif // BATTLECOMBATANTFRAME_H
