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
    explicit DiceRollDialog(QWidget *parent = nullptr);
    explicit DiceRollDialog(const Dice& dice, QWidget *parent = nullptr);
    ~DiceRollDialog();

    void fireAndForget();

public slots:
    void rollDice();

protected:
    virtual void hideEvent(QHideEvent * event);

private:
    void init();
    void rollDiceString(int& resultValue, QString& resultString);
    void rollDiceSpecified(int& resultValue, QString& resultString);
    void rollOnce(const Dice& dice, int& resultValue, QString& resultString);

    Ui::DiceRollDialog *ui;
    bool _fireAndForget;

    bool _mouseDown;
    QPoint _mouseDownPos;
};

#endif // DICEROLLDIALOG_H
