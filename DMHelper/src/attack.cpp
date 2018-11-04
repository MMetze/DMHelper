#include "attack.h"

Attack::Attack(const QString& attackName, const QString& diceString) :
    _name(attackName),
    _dice(diceString)
{
}

Attack::Attack(const QString& attackName, const Dice& dice) :
    _name(attackName),
    _dice(dice)
{
}

Attack::Attack(const Attack& obj) :
    _name(obj._name),
    _dice(obj._dice.toString())
{
}

const QString& Attack::getName() const
{
    return _name;
}

const Dice& Attack::getDice() const
{
    return _dice;
}
