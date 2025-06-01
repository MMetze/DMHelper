#include "dice.h"
#include <QRegularExpression>
#include <QRandomGenerator>

Dice::Dice() :
    _dieCount(0),
    _dieType(0),
    _dieBonus(0)
{
}

Dice::Dice(int dieCount, int dieType, int dieBonus) :
    _dieCount(dieCount),
    _dieType(dieType),
    _dieBonus(dieBonus)
{
}

Dice::Dice(const QString& diceString) :
    _dieCount(0),
    _dieType(0),
    _dieBonus(0)
{
    readString(diceString);
}

Dice::Dice(const Dice& other) :
    _dieCount(other._dieCount),
    _dieType(other._dieType),
    _dieBonus(other._dieBonus)
{
}

Dice& Dice::operator=(const Dice& other)
{
    _dieCount = other._dieCount;
    _dieType = other._dieType;
    _dieBonus = other._dieBonus;

    return *this;
}

bool Dice::isValid() const
{
    // Valid is defined as any die that can return a non-zero result
    return(((_dieType > 0) && (_dieCount > 0)) ||
           (_dieBonus > 0));
}

int Dice::getCount() const
{
    return _dieCount;
}

void Dice::setCount(int count)
{
    _dieCount = count;
}

int Dice::getType() const
{
    return _dieType;
}

void Dice::setType(int type)
{
    _dieType = type;
}

int Dice::getBonus() const
{
    return _dieBonus;
}

void Dice::setBonus(int bonus)
{
    _dieBonus = bonus;
}

QString Dice::toString() const
{
    if(((_dieCount == 0) || (_dieType == 0)) && (_dieBonus == 0))
        return QString("---");

    QString result;

    result = QString::number(getCount());
    result.append(QString("d"));
    result.append(QString::number(getType()));
    if(getBonus() != 0)
    {
        if(getBonus() >= 0)
            result.append(QString("+"));
        result.append(QString::number(getBonus()));
    }

    return result;
}

int Dice::roll()
{
    // Go through and roll the dice
    int result = 0;
    for(int dc = 0; dc < _dieCount; ++dc)
    {
        result += 1 + QRandomGenerator::global()->bounded(_dieType > 0 ? _dieType : 1);
    }

    result += _dieBonus;

    return result;
}

int Dice::average()
{
    // Go through and take the average of each die
    int result = 0;
    for(int dc = 0; dc < _dieCount; ++dc)
    {
        result += 1 + _dieType; // add min value and max value
    }

    // Take the average
    result /= 2;

    // Add any bonus
    result += _dieBonus;

    return result;
}

int Dice::d4()
{
    return 1 + QRandomGenerator::global()->bounded(4);
}

int Dice::d6()
{
    return 1 + QRandomGenerator::global()->bounded(6);
}

int Dice::d8()
{
    return 1 + QRandomGenerator::global()->bounded(8);
}

int Dice::d10()
{
    return 1 + QRandomGenerator::global()->bounded(10);
}

int Dice::d12()
{
    return 1 + QRandomGenerator::global()->bounded(12);
}

int Dice::d20()
{
    return 1 + QRandomGenerator::global()->bounded(20);
}

int Dice::d100()
{
    return 1 + QRandomGenerator::global()->bounded(100);
}

int Dice::dX(int X)
{
    return 1 + QRandomGenerator::global()->bounded(X);
}

void Dice::readString(const QString& diceString)
{
    QString expression("(\\d*)d(\\d+)(\\+|-)*(\\d*)");
    QRegularExpression re(expression);
    QRegularExpressionMatch match = re.match(diceString);
    if(match.hasMatch())
    {
        int caps = match.lastCapturedIndex();
        _dieCount = match.captured(1).toInt();
        if(_dieCount == 0)
            _dieCount = 1;
        _dieType = match.captured(2).toInt();
        if(caps > 2)
        {
            _dieBonus = match.captured(4).toInt();
            if(match.captured(3) == QString("-"))
            {
                _dieBonus *= -1;
            }
        }
    }
    else
    {
        bool ok = false;
        int diceInt = diceString.toInt(&ok);
        if(ok)
            _dieBonus = diceInt;
        else
            _dieCount = _dieType = _dieBonus = 0;
    }
}
