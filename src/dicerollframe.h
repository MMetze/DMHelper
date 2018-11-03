#ifndef DICEROLLFRAME_H
#define DICEROLLFRAME_H

#include <QFrame>
#include "dice.h"

namespace Ui {
class DiceRollFrame;
}

class DiceRollFrame : public QFrame
{
    Q_OBJECT

public:
    explicit DiceRollFrame(QWidget *parent = 0);
    explicit DiceRollFrame(const Dice& dice, QWidget *parent = 0);
    ~DiceRollFrame();

public slots:
    void rollDice();

private:
    void init();

    Ui::DiceRollFrame *ui;
};

#endif // DICEROLLFRAME_H
