#include "basicdate.h"
#include <QStringList>

const int DaysPerMonth[12] = {
    31,   // January
    28,   // February
    31,   // March
    30,   // April
    31,   // May
    30,   // June
    31,   // July
    31,   // August
    30,   // September
    31,   // October
    30,   // November
    31,   // December
};

const int DaysUntilMonth[12] = {
    0,   // January
    31,   // February
    59,   // March
    90,   // April
   120,   // May
   151,   // June
   181,   // July
   212,   // August
   243,   // September
   273,   // October
   304,   // November
   334,   // December
};

BasicDate::BasicDate(int d, int m, int y) :
    _day(d),
    _month(m),
    _year(y)
{
}

BasicDate::BasicDate(const QString &ddmmyyyy) :
    _day(0),
    _month(0),
    _year(0)
{
    fromStringDDMMYYYY(ddmmyyyy);
}

BasicDate::BasicDate(const BasicDate &d) :
    _day(d._day),
    _month(d._month),
    _year(d._year)
{
}

BasicDate::~BasicDate()
{
}

bool BasicDate::isValid() const
{
    if((_month < 1) ||
       (_month > 12) ||
       (_day < 1) ||
       (_day > daysInMonth()))
    {
        return false;
    }
    else
    {
        return true;
    }
}

void BasicDate::addDay()
{
    ++_day;
    if(_day > daysInMonth())
    {
        _day = 1;
        addMonths(1);
    }
}

void BasicDate::addMonths(int nmonths)
{
    addYears(nmonths / 12);
    _month += nmonths % 12;
    if(_month > 12)
    {
        _month -= 12;
        ++_year;
    }
}

void BasicDate::addYears(int nyears)
{
    _year += nyears;
}

int BasicDate::day() const
{
    return _day;
}

int BasicDate::month() const
{
    return _month;
}

int BasicDate::year() const
{
    return _year;
}

int BasicDate::daysInMonth() const
{
    if((_month < 1) || (_month > 12))
        return 0;
    else
        return DaysPerMonth[_month-1];
}

int BasicDate::daysUntilMonth() const
{
    if((_month < 1) || (_month > 12))
        return 0;
    else
        return DaysUntilMonth[_month-1];
}

int BasicDate::dayOfYear() const
{
    return daysUntilMonth() + _day;
}

int BasicDate::daysInYear() const
{
    return 365;
}

void BasicDate::getDate(int *year, int *month, int *day) const
{
    if(day)
        *day = _day;

    if(month)
        *month = _month;

    if(year)
        *year = _year;
}

void BasicDate::setDate(int year, int month, int day)
{
    _day = day;
    _month = month;
    _year = year;
}

QString BasicDate::toStringDDMMYYYY() const
{
    QString result = QString::number(_day).rightJustified(2,QChar('0'));
    result.append(QString("/"));
    result.append(QString::number(_month).rightJustified(2,QChar('0')));
    result.append(QString("/"));
    result.append(QString::number(_year).rightJustified(4,QChar('0')));
    return result;
}

void BasicDate::fromStringDDMMYYYY(const QString &ddmmyyyy)
{
    QStringList parts = ddmmyyyy.split(QString("/"));
    if(parts.count() != 3)
        return;

    _day = parts.at(0).toInt();
    _month = parts.at(1).toInt();
    _year = parts.at(2).toInt();
}

BasicDate& BasicDate::operator=(const BasicDate &d)
{
    _day = d._day;
    _month = d._month;
    _year = d._year;

    return *this;
}

bool BasicDate::operator!=(const BasicDate &d) const
{
    return !(*this == d);
}

bool BasicDate::operator<(const BasicDate &d) const
{
    if(_year < d.year())
        return true;

    if(_year > d.year())
        return false;

    // Same year
    if(_month < d.month())
        return true;

    if(_month > d.month())
        return false;

    // Same month & year
    if(_day < d.day())
        return true;
    else
        return false;
}

bool BasicDate::operator<=(const BasicDate &d) const
{
    return !(*this > d);
}

bool BasicDate::operator==(const BasicDate &d) const
{
    return((_day == d.day()) &&
           (_month == d.month()) &&
           (_year == d.year()));
}

bool BasicDate::operator>(const BasicDate &d) const
{
    if(_year > d.year())
        return true;

    if(_year < d.year())
        return false;

    // Same year
    if(_month > d.month())
        return true;

    if(_month < d.month())
        return false;

    // Same month & year
    if(_day > d.day())
        return true;
    else
        return false;
}

bool BasicDate::operator>=(const BasicDate &d) const
{
    return !(*this < d);
}
