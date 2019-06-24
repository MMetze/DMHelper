#include "basicdate.h"
#include "basicdateserver.h"
#include <QStringList>

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
    if((_day < 1) ||
       (_day > daysInMonth()) ||
       (_month < 1) ||
       (_month > monthsInYear()))
        return false;
    else
        return true;
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
    if((nmonths <= 0) || (!BasicDateServer::Instance()))
        return;

    int yearMonths = BasicDateServer::Instance()->getMonthsInYear(_year);
    if(yearMonths <= 0)
        return;

    while(nmonths >= yearMonths)
    {
        nmonths -= yearMonths;
        ++_year;
        yearMonths = BasicDateServer::Instance()->getMonthsInYear(_year);
        if(yearMonths <= 0)
            return;
    }

    _month += nmonths;
    if(_month > yearMonths)
    {
        _month -= yearMonths;
        ++_year;
    }

    while(BasicDateServer::Instance()->getDaysInMonth(_month, _year) == 0)
    {
        if(++_month > yearMonths)
        {
            _month -= yearMonths;
            ++_year;
        }
    }

    /*
    addYears(nmonths / 12);
    _month += nmonths % 12;
    if(_month > 12)
    {
        _month -= 12;
        ++_year;
    }
    */
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
    if(!BasicDateServer::Instance())
        return 0;

    if((_month < 1) || (_month > BasicDateServer::Instance()->getMonthsInYear(_year)))
        return 0;
    else
        return BasicDateServer::Instance()->getDaysInMonth(_month, _year);
}

int BasicDate::daysUntilMonth() const
{
    if(!BasicDateServer::Instance())
        return -1;

    if((_month < 1) || (_month > 12))
        return -1;
    else
        return BasicDateServer::Instance()->getDaysBeforeMonth(_month, _year);
}

int BasicDate::dayOfYear() const
{
    return daysUntilMonth() + _day;
}

int BasicDate::monthsInYear() const
{
    if(!BasicDateServer::Instance())
        return -1;

    return BasicDateServer::Instance()->getMonthsInYear(_year);
}

int BasicDate::daysInYear() const
{
    if(!BasicDateServer::Instance())
        return -1;

    return BasicDateServer::Instance()->getDaysInYear(_year);
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
