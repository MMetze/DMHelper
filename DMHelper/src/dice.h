#ifndef DICE_H
#define DICE_H

#include <QString>
#include <QMetaType>

class Dice
{
public:
    explicit Dice();
    explicit Dice(int dieCount, int dieType, int dieBonus);
    explicit Dice(const QString& diceString);
    Dice(const Dice& other);

    Dice& operator=(const Dice& other);

    bool isValid() const;

    int getCount() const;
    void setCount(int count);

    int getType() const;
    void setType(int type);

    int getBonus() const;
    void setBonus(int bonus);

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

    // Convenience parsers for dice expressions provided as strings (e.g.
    // "2d8+4"). Each constructs a temporary Dice from the expression and
    // returns the corresponding value. They return 0 for unparseable input,
    // matching the behaviour of Dice::Dice(const QString&) on failure.
    static int roll(const QString& expression);
    static int maximum(const QString& expression);
    static int average(const QString& expression);

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

Q_DECLARE_METATYPE(Dice);

#endif // DICE_H
