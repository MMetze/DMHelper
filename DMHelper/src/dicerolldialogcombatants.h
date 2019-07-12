#ifndef DICEROLLDIALOGCOMBATANTS_H
#define DICEROLLDIALOGCOMBATANTS_H

#include <QDialog>
#include <QStringList>
#include <QVBoxLayout>
#include "dice.h"
#include "battledialogmodel.h"

namespace Ui {
class DiceRollDialogCombatants;
}

class WidgetBattleCombatant;

class DiceRollDialogCombatants : public QDialog
{
    Q_OBJECT

public:
    //explicit DiceRollDialogCombatants(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, const QList<int>& modifiers, int rollDC = 10, QWidget *parent = 0);
    explicit DiceRollDialogCombatants(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, int rollDC = 10, QWidget *parent = nullptr);
    ~DiceRollDialogCombatants();

    void fireAndForget();

signals:
    void selectCombatant(BattleDialogModelCombatant* combatant);
    void combatantChanged(BattleDialogModelCombatant* combatant);

public slots:
    void rollDice();
    void applyDamage();
    void rerollWidget(WidgetBattleCombatant* widget);
    void setWidgetVisibility();

protected:
    virtual void hideEvent(QHideEvent * event);

private slots:
    void diceTypeChanged();
    void modifierTypeChanged();

private:
    void init();
    void createCombatantWidgets();

    int rollOnce(const Dice& dice, int modifier, QString& resultStr);
    void rollForWidget(WidgetBattleCombatant* widget, const Dice& dice, int modifier);
    Dice readDice();

    Ui::DiceRollDialogCombatants *ui;
    QVBoxLayout* _combatantLayout;

    QList<BattleDialogModelCombatant*> _combatants;
    QList<int> _modifiers;
    bool _fireAndForget;

    bool _mouseDown;
    QPoint _mouseDownPos;
};

#endif // DICEROLLDIALOGCOMBATANTS_H
