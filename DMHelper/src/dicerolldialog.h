#ifndef DICEROLLDIALOG_H
#define DICEROLLDIALOG_H

#include <QDialog>
#include <QStringList>
#include "dice.h"
#include "battledialogmodel.h"

namespace Ui {
class DiceRollDialog;
}

class DiceRollDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiceRollDialog(QWidget *parent = 0);
    explicit DiceRollDialog(const Dice& dice, QWidget *parent = 0);
    //explicit DiceRollDialog(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, int rollDC = 10, QWidget *parent = 0);
    //explicit DiceRollDialog(const Dice& dice, const QList<BattleDialogModelCombatant*>& combatants, const QList<int>& modifiers, int rollDC = 10, QWidget *parent = 0);
    ~DiceRollDialog();

    void fireAndForget();

/*
signals:
    void selectCombatant(BattleDialogModelCombatant*);
*/

public slots:
    void rollDice();

protected:
    virtual void hideEvent(QHideEvent * event);
    //virtual bool eventFilter(QObject *obj, QEvent *event);

/*
private slots:
    void diceTypeChanged();
    void modifierTypeChanged();
*/

private:
    void init();

    Ui::DiceRollDialog *ui;
    //QList<BattleDialogModelCombatant*> _combatants;
    //QList<int> _modifiers;
    bool _fireAndForget;

    bool _mouseDown;
    QPoint _mouseDownPos;
};

#endif // DICEROLLDIALOG_H
