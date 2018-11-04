#ifndef ATTACK_H
#define ATTACK_H

#include <QString>
#include "dice.h"

class Attack
{
public:
    explicit Attack(const QString& attackName, const QString& diceString);
    explicit Attack(const QString& attackName, const Dice& dice);
    explicit Attack(const Attack& obj);

    const QString& getName() const;
    const Dice& getDice() const;

protected:
    QString _name;
    Dice _dice;

};

#endif // ATTACK_H
