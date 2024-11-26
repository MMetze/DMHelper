#ifndef DICEROLLDIALOGCOMBATANTS_H
#define DICEROLLDIALOGCOMBATANTS_H

#include <QDialog>
#include <QStringList>
#include <QVBoxLayout>
#include "dice.h"
#include "combatant.h"

namespace Ui {
class DiceRollDialogCombatants;
}

class BattleCombatantWidget;
class BattleDialogModelCombatant;

class DiceRollDialogCombatants : public QDialog
{
    Q_OBJECT

public:
    explicit DiceRollDialogCombatants(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, int rollDC = 10, QWidget *parent = nullptr);
    ~DiceRollDialogCombatants();

    void fireAndForget();

    void setSaveDC(int saveDC);
    void setSaveType(const QString& saveType);
    void setConditions(int conditions);
    void setDamage(int damage);
    void setDamageDice(const Dice& damageDice);

signals:
    void selectCombatant(BattleDialogModelCombatant* combatant);
    void combatantChanged(BattleDialogModelCombatant* combatant);
    void hitPointsChanged(BattleDialogModelCombatant* combatant, int change);
    void removeEffect();

public slots:
    void rollDice();
    void rollDamageDice();
    void rerollWidget(BattleCombatantWidget* widget);
    void setWidgetVisibility();

protected:
    virtual void hideEvent(QHideEvent * event);

private slots:
    void diceTypeChanged();
    void modifierTypeChanged();
    void editConditions();
    void applyEffect();
    void applyEffectandClose();
    void applyEffectandDelete();

private:
    void init();
    void createCombatantWidgets();
    void updateConditionLayout();
    void addCondition(Combatant::Condition condition);

    int rollOnce(const Dice& dice, int modifier, QString& resultStr);
    void rollForWidget(BattleCombatantWidget* widget, const Dice& dice, int modifier);
    Dice readDice();

    Ui::DiceRollDialogCombatants *ui;
    QVBoxLayout* _combatantLayout;

    QList<BattleDialogModelCombatant*> _combatants;
    QList<int> _modifiers;
    bool _fireAndForget;
    int _conditions;
    QGridLayout* _conditionGrid;

    bool _mouseDown;
    QPoint _mouseDownPos;
};

#endif // DICEROLLDIALOGCOMBATANTS_H
