#ifndef DICE_H
#define DICE_H

#include <QString>

class Dice
{
public:
    explicit Dice();
    explicit Dice(int dieCount, int dieType, int dieBonus);
    explicit Dice(const QString& diceString);
    Dice(const Dice& other);

    Dice& operator=(const Dice& other);

    int getCount() const;
    int getType() const;
    int getBonus() const;

    QString toString() const;

    int roll();
    int average();

    static int d4();
    static int d6();
    static int d8();
    static int d10();
    static int d12();
    static int d20();
    static int d100();
    static int dX(int X);

private:
    void readString(const QString& diceString);

    int _dieCount;
    int _dieType;
    int _dieBonus;
};

inline bool operator==(const Dice& lhs, const Dice& rhs)
{
    return ((lhs.getCount() == rhs.getCount()) &&
            (lhs.getType() == rhs.getType()) &&
            (lhs.getBonus() == rhs.getBonus()));
}

inline bool operator!=(const Dice& lhs, const Dice& rhs)
{
    return !(lhs == rhs);
}

#endif // DICE_H
